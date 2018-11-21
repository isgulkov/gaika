
#include "obj_io.hpp"

#include <fstream>
#include <algorithm>

#include <iostream> //

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
}

obj_file obj_file::read_file(std::string path)
{
    std::ifstream fin(path);

    obj_file obj;

    int n_ignored = 0;
    std::string word;

    while(fin >> word) {
        if(word == "v") {
            float x, y, z;

            fin >> x >> y >> z;

            obj.vertices.emplace_back(x, y, z);
        }
        else if(word == "f") {
            std::vector<uint32_t> ix_vertices;
            uint32_t i_vertex;

            while(fin >> i_vertex) {
                ix_vertices.push_back(i_vertex - 1);
            }

            fin.clear();

            if(ix_vertices.size() >= 3) {
                for(int i = 2; i < ix_vertices.size(); i++) {
                    obj.triangles.emplace_back(ix_vertices[0], ix_vertices[i - 1], ix_vertices[i]);
                }
            }
            else {
                std::cout << "Ignoring face, not enough vertices" << std::endl;
            }

            continue;
        }
        else if(word[0] == '#') {
            std::cout << "Ignoring comment" << std::endl;
        }
        else {
            std::cout << "Ignoring `" << word << "` line" << std::endl;
            n_ignored += 1;
        }

        consume_line(fin);
    }

    std::cout << "\nFile seems to have been read.\n"
              << "Vertices: " << obj.vertices.size() << ", "
              << "triangles: " << obj.triangles.size() << ", "
              << "ignored " << n_ignored << " lines." << std::endl;

    return obj;
}
