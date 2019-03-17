
#include "matrices.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

vec3f& vec3f::operator*=(float u)
{
    x *= u;
    y *= u;
    z *= u;

    return *this;
}

vec3f vec3f::operator*(float u) const
{
    return { x * u, y * u, z * u };
}

vec3f& vec3f::operator/=(float u)
{
    x /= u;
    y /= u;
    z /= u;

    return *this;
}

vec3f vec3f::operator/(float u) const
{
    return { x / u, y / u, z / u };
}

vec3f operator*(float u, const vec3f& v)
{
    return v.operator*(u);
}

vec3f vec3f::cross(const vec3f& other) const
{
    return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
    };
}

vec3f& vec3f::operator+=(const vec3f& other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

vec3f vec3f::operator+(const vec3f& other) const
{
    vec3f result = *this;

    return result += other;
}

vec3f vec3f::operator*(const vec3f& other) const
{
    return { x * other.x, y * other.y, z * other.z };
}

float vec3f::dot(const vec3f& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

vec3f& vec3f::operator-=(const vec3f& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}

vec3f vec3f::operator-(const vec3f& other) const
{
    vec3f result = *this;

    return result -= other;
}

vec3f vec3f::operator-() const
{
    return { -x, -y, -z };
}

float vec3f::norm() const
{
    return std::sqrt(x * x + y * y + z * z);
}

vec3f& vec3f::normalize()
{
    const float n = norm();

    return n != 0 ? operator*=(1.0f / n) : *this;
}

vec3f vec3f::unit() const
{
    const float n = norm();

    return n != 0 ? operator*(1.0f / n) : *this;
}

inline float clamp(float x, float low, float high)
{
    return x >= low ? (x < high ? x : (high - std::numeric_limits<float>::epsilon())) : low;
}

vec3f& vec3f::clamp(float low, float high)
{
    x = ::clamp(x, low, high);
    y = ::clamp(y, low, high);
    z = ::clamp(z, low, high);

    return *this;
}

vec3f vec3f::clamped(float low, float high) const
{
    return { ::clamp(x, low, high), ::clamp(y, low, high), ::clamp(z, low, high) };
}

std::string vec3f::to_string() const
{
    return (std::stringstream() << std::fixed << std::setprecision(2) << "(" << x << "," << y << "," << z << ")").str();
}

//

vec4f& vec4f::operator*=(float u)
{
    x *= u;
    y *= u;
    z *= u;
    w *= u;

    return *this;
}

vec4f vec4f::operator*(float u) const
{
    return { x * u, y * u, z * u, w * u };
}

vec4f& vec4f::operator+=(const vec4f& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}

vec4f vec4f::operator+(const vec4f& other) const
{
    return { x + other.x, y + other.y, z + other.z, w + other.w };
}

vec4f vec4f::operator-(const vec4f& other) const
{
    return { x - other.x, y - other.y, z - other.z, w - other.w };
}

vec3f vec4f::to_cartesian() const
{
    return { x / w, y / w, z / w };
}

std::string vec4f::to_string() const
{
    return (
            std::stringstream() << std::fixed << std::setprecision(2)
                                << "(" << x << "," << y << "," << z << "," << w << ")"
    ).str();
}

//

float mat_sq4f::at(int i_row, int j_col) const
{
    return rows[i_row][j_col];
}

mat_sq4f mat_sq4f::identity()
{
    return {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat_sq4f mat_sq4f::operator*(const mat_sq4f& other) const
{
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
    const float x = rows[0][0] * v.x + rows[0][1] * v.y + rows[0][2] * v.z + rows[0][3];
    const float y = rows[1][0] * v.x + rows[1][1] * v.y + rows[1][2] * v.z + rows[1][3];
    const float z = rows[2][0] * v.x + rows[2][1] * v.y + rows[2][2] * v.z + rows[2][3];

    const float w = rows[3][0] * v.x + rows[3][1] * v.y + rows[3][2] * v.z + rows[3][3];

    return { x / w, y / w, z / w };
}

vec4f mat_sq4f::mul_homo(const vec3f& v) const
{
    const float x = rows[0][0] * v.x + rows[0][1] * v.y + rows[0][2] * v.z + rows[0][3];
    const float y = rows[1][0] * v.x + rows[1][1] * v.y + rows[1][2] * v.z + rows[1][3];
    const float z = rows[2][0] * v.x + rows[2][1] * v.y + rows[2][2] * v.z + rows[2][3];
    const float w = rows[3][0] * v.x + rows[3][1] * v.y + rows[3][2] * v.z + rows[3][3];

    return { x, y, z, w };
}

std::vector<vec3f> mat_sq4f::operator*(const std::vector<vec3f>& vs) const
{
    std::vector<vec3f> new_vs(vs.size());

    std::transform(vs.begin(), vs.end(), new_vs.begin(), [this](const vec3f& v){ return operator*(v); });

    return new_vs;
}

std::vector<vec4f> mat_sq4f::mul_homo(const std::vector<vec3f>& vs) const
{
    std::vector<vec4f> new_vs(vs.size());

    std::transform(vs.begin(), vs.end(), new_vs.begin(), [this](const vec3f& v){ return mul_homo(v); });

    return new_vs;
}

void mat_sq4f::multiply_row(int i_row, float x)
{
    for(int i_col = 0; i_col < 4; i_col++) {
        rows[i_row][i_col] *= x;
    }
}

void mat_sq4f::subtract_rows(int i_row, int j_row, float x)
{
    for(int i_col = 0; i_col < 4; i_col++) {
        rows[i_row][i_col] -= rows[j_row][i_col] * x;
    }
}

mat_sq4f mat_sq4f::transpose() const
{
    return {
        rows[0][0], rows[1][0], rows[2][0], rows[3][0],
        rows[0][1], rows[1][1], rows[2][1], rows[3][1],
        rows[0][2], rows[1][2], rows[2][2], rows[3][2],
        rows[0][3], rows[1][3], rows[2][3], rows[3][3]
    };
}

mat_sq4f mat_sq4f::inverse() const
{
    mat_sq4f a = *this, b = identity();

    for(int i = 0; i < 4; i++) {
        const float x_diag = a.rows[i][i];
        a.multiply_row(i, 1.0f / x_diag);
        b.multiply_row(i, 1.0f / x_diag);

        for(int j = 0; j < 4; j++) {
            if(i == j) {
                continue;
            }

            const float x_col = a.rows[j][i];
            a.subtract_rows(j, i, x_col);
            b.subtract_rows(j, i, x_col);
        }
    }

    return b;
}

std::string mat_sq4f::to_string() const
{
    std::stringstream ss;

    ss << "[" << std::fixed << std::setprecision(2);

    for(const std::array<float, 4>& row : rows) {
        ss << "(";
        bool comma = false;

        for(float x : row) {
            if(comma) {
                ss << ",";
            }
            else {
                comma = true;
            }

            ss << x;
        }

        ss << ")";
    }

    ss << "]";

    return ss.str();
}
