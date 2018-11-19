
#ifndef DZ_GAIKA_GEOMETRY_HPP
#define DZ_GAIKA_GEOMETRY_HPP

#include "matrices.hpp"

namespace mx_tx {

mat_sq4f scale(float s);
mat_sq4f scale_xyz(float sx, float sy, float sz);
mat_sq4f translate(const vec3f& xs);
mat_sq4f rotate_x(float theta);
mat_sq4f rotate_y(float theta);
mat_sq4f rotate_z(float theta);
mat_sq4f rotate_xyz(const vec3f& thetas);
mat_sq4f rotate_zyx(const vec3f& thetas);
mat_sq4f project_ortho_x();
mat_sq4f project_ortho_y();
mat_sq4f project_ortho_z();
mat_sq4f project_perspective_z(float theta_w, float theta_h);
mat_sq4f project_perspective_z(float theta_w, float theta_h, float z_near, float z_far);

vec3f rotate_to_eul(const mat_sq4f& mx_rotate);

}

#endif //DZ_GAIKA_GEOMETRY_HPP
