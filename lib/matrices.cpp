
#include "matrices.hpp"

#include <vector>
#include <algorithm>

template <typename T, size_t N>
std::ostream& print_array(std::ostream& os, const std::array<T, N>& xs)
{
    // TODO: move all this print shit into a separate module

    bool prepend_comma = false;

    for(T x : xs) {
        if(prepend_comma) {
            os << ", ";
        }
        else {
            prepend_comma = true;
        }

        os << x;
    }

    return os;
}

//

vec3f& vec3f::operator*=(const vec3f& other)
{
    for(int i = 0; i < 3; i++) {
        _xs[i] *= other._xs[i];
    }

    return *this;
}

vec3f vec3f::operator*(const vec3f& other) const
{
    vec3f product = *this;

    return product *= other;
}

int16_t what_do_i_call_this(float x, int16_t x_size)
{
    return (int16_t)(((x + 1.0f) / 5.0f) * x_size);
}

vec2s vec3f::onto_xy_screen(int16_t x_size, int16_t y_size) const
{
    // TODO: now, how do I call this and how should it be implemented?
    return { what_do_i_call_this(x(), x_size), what_do_i_call_this(y(), y_size) };
}

std::ostream& operator<<(std::ostream& os, const vec3f& v)
{
    return print_array(os << "(", v.xs()) << ")";
}

mat_sq4f mat_sq4f::operator*(const mat_sq4f& other) const
{
    // TODO: represent as std::array<float, 16>; multiply with std::transform (or something)

    std::array<std::array<float, 4>, 4> new_rows;

    for(int i_row = 0; i_row < 4; i_row++) {
        for(int i_col = 0; i_col < 4; i_col++) {
            new_rows[i_row][i_col] = 0.0f;

            for(int i = 0; i < 4; i++) {
                new_rows[i_row][i_col] += rows[i_row][i] * other.rows[i][i_col];
            }
        }
    }

    return { new_rows };
}

vec3f mat_sq4f::operator*(const vec3f& v) const
{
    std::array<float, 3> new_xs;

    for(size_t i = 0; i < 3; i++) {
        new_xs[i] = rows[i][3];

        for(size_t j = 0; j < 3; j++) {
            new_xs[i] += rows[i][j] * v.at(j);
        }
    }

//        return { new_xs };
    return { new_xs[0], new_xs[1], new_xs[2] };
}

std::vector<vec3f> mat_sq4f::operator*(const std::vector<vec3f>& vs) const
{
    std::vector<vec3f> new_vs(vs.size());

    std::transform(vs.begin(), vs.end(), new_vs.begin(), [this](const vec3f& v){ return operator*(v); });

//        for(size_t i = 0; i < vs.size(); i++) {
//            new_vs[i] = operator*(vs[i]);
//        }

    return new_vs;
}

std::ostream& operator<<(std::ostream& os, const mat_sq4f& m)
{
    os << "[";

    bool prepend_comma = false;

    for(const std::array<float, 4>& row : m.rows) {
        if(prepend_comma) {
            os << ", ";
        }
        else {
            prepend_comma = true;
        }

        print_array(os << "(", row) << ")";
    }

    return os << "]";
}
