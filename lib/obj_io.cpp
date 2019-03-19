
#include "obj_io.hpp"
#include "model.hpp"

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

void consume_line(std::istream& is)
{
    char c;

    while(is.get(c)) {
        if(c == '\n') {
            return;
        }
    }
}

void consume_ws(std::istream& is)
{
    char c = '\0';

    while(is.get(c) && (c == ' ' || c == '\t')) { }

    if(c) is.putback(c);
}

std::string read_word(std::istream& is)
{
    consume_ws(is);

    char c;

    if(!is.get(c)) {
        return "";
    }
    else if(c == '\r' || c == '\n') {
        is.putback(c);
        return "";
    }

    std::string word;

    do {
        word += c;
    } while(is.get(c) && c != ' ' && c != '\r' && c != '\n');

    if(c == '\r' || c == '\n') {
        is.putback(c);
    }

    return word;
}

using namespace isg;

vec3f read_color(const std::string& s)
{
    std::istringstream ss(s);

    float x, y, z;

    ss >> x >> y >> z;

    return { x, y, z };
}

std::unordered_map<std::string, material> read_mtllib(const std::string& path)
{
    std::ifstream fin(path);

    std::unordered_map<std::string, material> mtls;

    std::string mtl_name;
    material mtl;

    int n_line = 0;
    std::string line;

    while(std::getline(fin, line)) {
        n_line += 1;

        size_t i_space = 0;

        while(i_space < line.size() && line[i_space] == ' ') {
            i_space += 1;
        }

        i_space = line.find_first_of(' ', i_space);

        if(i_space == std::string::npos) {
            continue;
        }

        const std::string first_word = line.substr(0, i_space);
        const std::string rest = line.substr(i_space + 1);

        if(first_word == "newmtl") {
            if(!mtl_name.empty()) {
                mtls[mtl_name] = mtl;
            }

            mtl_name = rest;
        }
        else if(first_word == "Kd") {
            mtl.c_diffuse = read_color(rest);
        }
        else if(first_word == "Ka") {
            mtl.c_ambient = read_color(rest);
            mtl.has_ambient = true;
        }
        else if(first_word == "Ks") {
            mtl.c_specular = read_color(rest);
        }
        else if(first_word == "Ns") {
            std::istringstream ss(rest);

            ss >> mtl.exp_specular;
        }
        else {
            std::cerr << "Line " << n_line << ": Ignoring `" << first_word << "` line\n";
        }
    }

    if(!mtl_name.empty()) {
        mtls[mtl_name] = mtl;
    }

    return mtls;
}

namespace isg
{
namespace obj_io
{
model read_obj_model(std::string path)
{
    std::ifstream fin(path);

    std::cerr << "Loading OBJ: \x1b[1m" << path << "\x1b[0m\n";

    model m;
    m.materials.emplace_back();

    std::unordered_map<std::string, size_t> ix_mtls;
    size_t i_mtl = 0;

    int n_ignored = 0, n_line = 0;

    std::string first_word;

    while(fin >> first_word) {
        n_line += 1;

        if(first_word == "o") {
            if(!m.name.empty()) {
                std::cerr << "Line " << n_line << ": Another `o` found, ignoring\n";
            }

            fin.get();
            std::getline(fin, m.name);

            fin.putback('\n');
        }
        else if(first_word[0] == '#') {
            std::cerr << "Line " << n_line << ": Ignoring comment\n";
        }
        else if(first_word == "mtllib") {
            std::string mtl_filename;
            fin >> mtl_filename;

            std::cerr << "Line " << n_line << ": Reading mtllib '" << mtl_filename << "'\n";
            auto new_mtls = read_mtllib(path.substr(0, path.rfind('/')) + "/" + mtl_filename);

            std::cerr << "It contained " << new_mtls.size() << " materials.\n";

            for(auto& kv : new_mtls) {
                ix_mtls[kv.first] = m.materials.size();
                m.materials.emplace_back(std::move(kv.second));
            }
        }
        else if(first_word == "usemtl") {
            std::string mtl_name;
            fin >> mtl_name;

            const auto it = ix_mtls.find(mtl_name);

            if(it == ix_mtls.end()) {
                std::cerr << "Line " << n_line << ": Unknown material '" << mtl_name << "'\n";
            }
            else {
                i_mtl = it->second;
            }
        }
        else if(first_word == "v" || first_word == "vn") {
            float x, y, z;

            std::string rest;
            std::getline(fin, rest);

            fin.putback('\n');

            std::istringstream ss(rest);
            ss >> x >> y >> z;

            if(first_word == "v") {
                m.vertices.emplace_back(x, y, z);
            }
            else if(first_word == "vn") {
                m.normals.emplace_back(x, y, z);
            }
        }
        else if(first_word == "f") {
            std::vector<size_t> ix_vertices;
            std::vector<size_t> ix_normals;

            std::string word;

            while(!(word = read_word(fin)).empty()) {
                size_t i_slash = word.find_first_of('/');

                try {
                    ix_vertices.push_back(std::stoll(word.substr(0, i_slash)) - 1);
                }
                catch(const std::invalid_argument& ex) { }

                // TODO: parse texture coords index as well (and refactor this three-index shit overall)

                if(std::count(word.begin(), word.end(), '/') != 2) {
                    ix_normals.push_back(SIZE_T_MAX);
                    continue;
                }

                i_slash = word.find_last_of('/');

                if(i_slash < word.size() - 1) {
                    ix_normals.push_back(std::stoll(word.substr(i_slash + 1)) - 1);
                }
                else {
                    ix_normals.push_back(SIZE_T_MAX);
                }
            }

            if(ix_vertices.size() >= 3) {
                for(int i = 2; i < ix_vertices.size(); i++) {
                    m.faces.emplace_back(
                            ix_vertices[0], ix_vertices[i - 1], ix_vertices[i],
                            ix_normals[0], ix_normals[i - 1], ix_normals[i],
                            i_mtl
                    );
                }
            }
            else {
                std::cerr << "Line " << n_line << ": Ignoring face, not enough vertices\n";
            }
        }
        else {
            std::cerr << "Line " << n_line << ": Ignoring `" << first_word << "` line\n";
            n_ignored += 1;
        }

        consume_line(fin);
    }

    if(m.name.empty()) {
        const std::string filename = path.substr(path.rfind('/') + 1);

        std::cerr << "No object name (`o`) statement encountered -- using '" << filename << "' instead.\n";

        m.name = filename;
    }

    std::cerr << "\x1b[1mDone.\x1b[0m\n"
              << "Vertices: " << m.vertices.size() << ", "
              << "normals: " << m.normals.size() << ", "
              << "faces: " << m.faces.size() << ", "
              << "ignored " << n_ignored << " lines out of " << n_line << ".\n\n";

    return m;
}
}
}
