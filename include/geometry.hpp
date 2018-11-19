
#ifndef DZ_GAIKA_GEOMETRY_HPP
#define DZ_GAIKA_GEOMETRY_HPP

#include "matrices.hpp"

namespace mx_tx {

mat_sq4f scale(float s);
mat_sq4f translate(const vec3f& xs);
mat_sq4f rotate_x(float theta);
mat_sq4f rotate_y(float theta);
mat_sq4f rotate_z(float theta);
mat_sq4f rotate_xyz(const vec3f& thetas);
mat_sq4f perspective_z(float theta_w, float wh_ratio);
mat_sq4f perspective_z(float theta_w, float wh_ratio, float z_near, float z_far);

}

#endif //DZ_GAIKA_GEOMETRY_HPP
