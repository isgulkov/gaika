
#ifndef DZ_GAIKA_GEOMETRY_HPP
#define DZ_GAIKA_GEOMETRY_HPP

#include "matrices.hpp"

namespace mx_tx {

mat_sq4f identity(); // REVIEW: should this be here?
mat_sq4f translate(vec3f xs);
mat_sq4f rot_x(float theta);
mat_sq4f rot_y(float theta);
mat_sq4f rot_z(float theta);
mat_sq4f perspective_z(float theta_w, float wh_ratio, float z_near, float z_far);

}

#endif //DZ_GAIKA_GEOMETRY_HPP
