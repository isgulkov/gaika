
#include "geometry.hpp"

#include <cmath>

namespace mx_tx {

mat_sq4f identity()
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f translate(vec3f xs)
{
    return {
            1.0f, 0.0f, 0.0f, xs.x(),
            0.0f, 1.0f, 0.0f, xs.y(),
            0.0f, 0.0f, 1.0f, xs.z(),
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rot_x(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, t_cos, -t_sin, 0.0f,
            0.0f, t_sin, t_cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rot_y(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            t_cos, 0.0f, t_sin, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -t_sin, 0.0f, t_cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f rot_z(float theta)
{
    const float t_cos = std::cos(theta), t_sin = std::sin(theta);

    return {
            t_cos, -t_sin, 0.0f, 0.0f,
            t_sin, t_cos, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f perspective_z(float theta_w, float wh_ratio, float z_near, float z_far)
{
    return {
        1.0f / std::tan(theta_w), 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / std::tan(theta_w / wh_ratio), 0.0f, 0.0f,
        0.0f, 0.0f, z_far / (z_far - z_near), 0.0f,
        0.0f, 0.0f, z_near * z_far / (z_near - z_far), 0.0f
    };
}

}
