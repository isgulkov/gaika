
#include "obj_io.hpp"

#include <fstream>
#include <unordered_map>
#include <iostream>

namespace {
void consume_line(std::istream& is)
{
    char c;

    while(is.get(c)) {
        if(c == '\n') {
            return;
        }
    }
}

//std::unordered_map<std::string, std::tuple<float, float, float>> read_mtllib(const std::string& path)
std::unordered_map<std::string, std::array<uint8_t, 3>> read_mtllib(const std::string& path)
{
    std::ifstream fin(path);

    // TODO: implement reading separate ambient/diffuse/specular colors when lighting is done
    std::unordered_map<std::string, std::array<uint8_t, 3>> mtl_colors;
    std::string mtl_name;

    std::string word;

    while(fin >> word) {
        if(word == "newmtl") {
            fin >> mtl_name;
        }
        else if(word == "Kd") {
            float r, g, b;
            fin >> r >> g >> b;

            if(mtl_name.empty()) {
                std::cerr << "Encountered Kd before any newmtl" << '\n';
            }
            else {
                mtl_colors.emplace(mtl_name, std::array<uint8_t, 3>{ (uint8_t)(255 * r), (uint8_t)(255 * g), (uint8_t)(255 * b) });
            }
        }
        else {
            std::cerr << "Ignoring `" << word << "`\n";
        }

        consume_line(fin);
    }

    return mtl_colors;
}
}

obj_file obj_file::read_file(std::string path)
{
    std::ifstream fin(path);

    std::cerr << "Loading OBJ: \x1b[1m" << path << "\x1b[0m\n";

    std::unordered_map<std::string, std::array<uint8_t, 3>> mtl_colors;
    std::array<uint8_t, 3> color = { 255, 255, 255 };

    obj_file obj;

    int n_ignored = 0, n_line = 1;
    std::string word;

    while(fin >> word) {
        if(word == "v") {
            float x, y, z;

            fin >> x >> y >> z;

            obj.vertices.emplace_back(x, y, z);
            obj.vertex_colors.emplace_back(color);
        }
        else if(word == "f") {
            std::vector<uint16_t> ix_vertices;
            uint16_t i_vertex;

            while(fin >> i_vertex) {
                ix_vertices.emplace_back(i_vertex - 1);
            }

            fin.clear();

            if(ix_vertices.size() >= 3) {
                for(int i = 2; i < ix_vertices.size(); i++) {
                    obj.triangles.emplace_back(ix_vertices[0], ix_vertices[i - 1], ix_vertices[i]);
                }
            }
            else {
                std::cerr << "Line " << n_line << ": Ignoring face, not enough vertices\n";
            }

            fin.putback('\n');
        }
        else if(word == "o") {
            fin.get();
            std::getline(fin, obj.name);

            fin.putback('\n');
        }
        else if(word == "mtllib") {
            std::string mtl_filename;
            fin >> mtl_filename;

            std::cerr << "Line " << n_line << ": Reading mtllib '" << mtl_filename << "'\n";
            auto new_colors = read_mtllib(path.substr(0, path.rfind('/')) + "/" + mtl_filename);

            std::cerr << "It contained " << new_colors.size() << " materials:\n";

            mtl_colors.insert(new_colors.begin(), new_colors.end());

        }
        else if(word == "usemtl") {
            std::string mtl_name;
            fin >> mtl_name;

            auto it = mtl_colors.find(mtl_name);

            if(it == mtl_colors.end()) {
                std::cerr << "Line " << n_line << ": Unknown material '" << mtl_name << "'\n";
            }
            else {
                std::cerr << "Line " << n_line << ": Found material '" << mtl_name << "'\n";
                color = it->second;
            }
        }
        else if(word[0] == '#') {
            std::cerr << "Line " << n_line << ": Ignoring comment\n";
        }
        else {
            std::cerr << "Line " << n_line << ": Ignoring `" << word << "` line\n";
            n_ignored += 1;
        }

        consume_line(fin);
        n_line += 1;
    }

    if(obj.name.empty()) {
        std::string filename = path.substr(path.rfind('/') + 1);

        std::cerr << "No object name (`o`) statement encountered -- using '" << filename << "' instead.\n";

        obj.name = filename;
    }

    std::cerr << "\x1b[1mDone.\x1b[0m\n"
              << "Vertices: " << obj.vertices.size() << ", "
              << "triangles: " << obj.triangles.size() << ", "
              << "ignored " << n_ignored << " lines out of " << n_line << ".\n\n";

    return obj;
}
