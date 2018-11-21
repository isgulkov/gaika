
#include "geometry.hpp"

#include <cmath>

namespace mx_tx {

namespace {
mat_sq4f diag(float a, float b, float c, float d)
{
    return {
            a, 0.0f, 0.0f, 0.0f,
            0.0f, b, 0.0f, 0.0f,
            0.0f, 0.0f, c, 0.0f,
            0.0f, 0.0f, 0.0f, d
    };
}
}

mat_sq4f scale(float s)
{
    return diag(1, 1, 1, 1.0f / s);
}

mat_sq4f scale(float sx, float sy, float sz)
{
    return diag(sx, sy, sz, 1);
}

mat_sq4f translate(float dx, float dy, float dz)
{
    return {
            1.0f, 0.0f, 0.0f, dx,
            0.0f, 1.0f, 0.0f, dy,
            0.0f, 0.0f, 1.0f, dz,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f translate(const vec3f& xs)
{
    return translate(xs.x, xs.y, xs.z);
}

mat_sq4f rotate_x(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, t_cos, -t_sin, 0.0f,
            0.0f, t_sin, t_cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rotate_y(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            t_cos, 0.0f, t_sin, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -t_sin, 0.0f, t_cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rotate_z(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            t_cos, -t_sin, 0.0f, 0.0f,
            t_sin, t_cos, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rotate_xyz(const vec3f& thetas)
{
    return rotate_z(thetas.z) * rotate_y(thetas.y) * rotate_x(thetas.x);
}

mat_sq4f rotate_zyx(const vec3f& thetas)
{
    return rotate_x(thetas.x) * rotate_y(thetas.y) * rotate_z(thetas.z);
}

mat_sq4f project_ortho_x()
{
    return diag(0, 1, 1, 1);
}

mat_sq4f project_ortho_y()
{
    return diag(1, 0, 1, 1);
}

mat_sq4f project_ortho_z()
{
    return diag(1, 1, 0, 1);
}

mat_sq4f project_perspective_z(float theta_w, float theta_h, float z_near, float z_far)
{
    return {
            1.0f / std::tan(theta_w / 2.0f), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / std::tan(theta_h / 2.0f), 0.0f, 0.0f,
            0.0f, 0.0f, -z_far / (z_far - z_near), -z_near * z_far / (z_far - z_near),
            0.0f, 0.0f, -1.0f, 0.0f
    };
}

vec3f rotate_to_eul(const mat_sq4f& mx_rotate)
{
    // https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Conversion_formulae_between_formalisms

    return {
            std::atan2f(mx_rotate.at(2, 0), mx_rotate.at(2, 1)),
            std::acos(mx_rotate.at(2, 2)),
            -std::atan2f(mx_rotate.at(0, 2), mx_rotate.at(1, 2))
    };
}

}
