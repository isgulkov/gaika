
#include "geometry.hpp"

#include <iostream>

namespace mx_transforms {

mat_sq4f rot_x(float theta)
{
    const float t_cos = std::cos(theta);
    const float t_sin = std::sin(theta);

    return {
            {
                    std::array<float, 4> { 1.0f, 0.0f, 0.0f, 0.0f },
                    std::array<float, 4> { 0.0f, t_cos, -t_sin, 0.0f },
                    std::array<float, 4> { 0.0f, t_sin, t_cos, 0.0f },
                    std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }
            }
    };
}

}
