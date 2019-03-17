
#ifndef DZ_GAIKA_MODEL_HPP
#define DZ_GAIKA_MODEL_HPP

#include <vector>
#include <string>

#include "matrices.hpp"

namespace isg
{
struct material
{
    vec3f c_ambient = { 1.0f, 1.0f, 1.0f };
    vec3f c_diffuse = { 1.0f, 1.0f, 1.0f };
    vec3f c_specular = { 1.0f, 1.0f, 1.0f };

    float exp_specular = 0;

    // TODO: corresponding texture maps
};

struct model
{
    struct segment
    {
        size_t i_a, i_b;
        size_t i_mtl;

        segment(size_t i_a, size_t i_b, size_t i_mtl = 0) : i_a(i_a), i_b(i_b), i_mtl(i_mtl) { }
    };

    struct face
    {
        size_t i_a, i_b, i_c;
        size_t in_a, in_b, in_c;
        size_t i_mtl;

        face(size_t i_a, size_t i_b, size_t i_c,
             size_t in_a, size_t in_b = SIZE_T_MAX, size_t in_c = SIZE_T_MAX,
             size_t i_mtl = 0) : i_a(i_a), i_b(i_b), i_c(i_c), in_a(in_a), in_b(in_b), in_c(in_c), i_mtl(i_mtl) { }
    };

    std::string name;

    std::vector<vec3f> vertices, normals;
    std::vector<material> materials;
    std::vector<segment> segments;
    std::vector<face> faces;

    static model tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
    {
        // In order A, B, C, P where (A, B, C) is counterclockwise when looking in P's direction

        return {
                "Tetrahedron",
                { a, b, c, p },
                { {} },
                { },
                {{ 0, 1, 2 }, { 1, 0, 3 }, { 2, 1, 3 }, { 0, 2, 3 }}
        };
    }
};
}

#endif //DZ_GAIKA_MODEL_HPP
