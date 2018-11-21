
#ifndef DZ_GAIKA_OBJ_IO_HPP
#define DZ_GAIKA_OBJ_IO_HPP

#include <array>
#include <vector>
#include <set>
#include <string>

#include "matrices.hpp"

struct obj_file
{
    std::vector<vec3f> vertices;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> triangles;

    // TODO: load colors from mtl files

    static obj_file read_file(std::string path);
};

#endif //DZ_GAIKA_OBJ_IO_HPP
