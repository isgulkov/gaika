
#ifndef DZ_GAIKA_MODEL_HPP
#define DZ_GAIKA_MODEL_HPP

#include <vector>
#include <string>

#include "matrices.hpp"

namespace isg
{
struct model
{
    struct segment_line
    {
        uint16_t i_a, i_b;
        uint8_t r, g, b, a;

        segment_line() = default;
        segment_line(uint16_t i_a, uint16_t i_b) : segment_line(i_a, i_b, 255, 255, 255) { }
        segment_line(uint16_t i_a, uint16_t i_b,
                std::array<uint8_t, 3> rgb) : segment_line(i_a, i_b, rgb[0], rgb[1], rgb[2]) { }
        segment_line(uint16_t i_a, uint16_t i_b,
                uint8_t r, uint8_t g, uint8_t b) : i_a(i_a), i_b(i_b), r(r), g(g), b(b), a(255) { }
    };

    struct triangle_face
    {
        uint16_t i_a, i_b, i_c;

        triangle_face() = default;
        triangle_face(uint16_t i_a, uint16_t i_b, uint16_t i_c) : i_a(i_a), i_b(i_b), i_c(i_c) { }
    };

    std::string name;

    std::vector<vec3f> vertices;
    std::vector<std::array<uint8_t, 3>> vertex_colors; // TODO: indices into "materials" array
    std::vector<segment_line> segments;
    std::vector<triangle_face> faces;

    static model tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
    {
        // In order A, B, C, P where (A, B, C) is counterclockwise when looking in P's direction

        return {
                "Tetrahedron",
                { a, b, c, p },
                { { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 }, { 255, 255, 255 } },
                { },
                {{ 0, 1, 2 }, { 1, 0, 3 }, { 2, 1, 3 }, { 0, 2, 3 }}
        };
    }
};
}

#endif //DZ_GAIKA_MODEL_HPP
