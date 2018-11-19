
#include "geometry.hpp"

#include <cmath>

namespace mx_tx {

mat_sq4f scale(float s)
{
    return {
            s, 0.0f, 0.0f, 0.0f,
            0.0f, s, 0.0f, 0.0f,
            0.0f, 0.0f, s, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f translate(const vec3f& xs)
{
    return {
            1.0f, 0.0f, 0.0f, xs.x(),
            0.0f, 1.0f, 0.0f, xs.y(),
            0.0f, 0.0f, 1.0f, xs.z(),
            0.0f, 0.0f, 0.0f, 1.0f
    };
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
    return rotate_x(thetas.x()) * rotate_y(thetas.y()) * rotate_z(thetas.z());
}

mat_sq4f perspective_z(float theta_w, float wh_ratio)
{
    return perspective_z(theta_w, wh_ratio, 0, 1);
}

mat_sq4f perspective_z(float theta_w, float wh_ratio, float z_near, float z_far)
{
    const float half_theta_w = theta_w / 2.0f;

    return {
            1.0f / std::tan(half_theta_w), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / std::tan(half_theta_w / wh_ratio), 0.0f, 0.0f,
            0.0f, 0.0f, z_far / (z_far - z_near), -1.0f,
            0.0f, 0.0f, -z_near * z_far / (z_far - z_near), 0.0f
    };
}

}
