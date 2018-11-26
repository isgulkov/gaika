
#ifndef DZ_GAIKA_OBJ_IO_HPP
#define DZ_GAIKA_OBJ_IO_HPP

#include <vector>
#include <string>

#include "matrices.hpp"
#include "model.hpp"

struct obj_file
{
    // TODO: short-circuit this file representation to the model class?
    // TODO: handle multiple models per file (and per object)?

    std::string name;

    std::vector<vec3f> vertices;
    std::vector<isg::model::triangle_face> triangles;

    static obj_file read_file(std::string path);
};

#endif //DZ_GAIKA_OBJ_IO_HPP
