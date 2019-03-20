
#ifndef DZ_GAIKA_MATRICES_HPP
#define DZ_GAIKA_MATRICES_HPP

#include <array>

// TODO: put each of the classes into a separate file

struct vec3f
{
    float x, y, z;

    vec3f(float x, float y, float z) : x(x), y(y), z(z) { }
    vec3f() : x(0), y(0), z(0) { }

    bool operator==(const vec3f& other) const;

    vec3f& operator*=(float u);
    vec3f operator*(float u) const;

    vec3f& operator/=(float u);
    vec3f operator/(float u) const;

    vec3f& operator+=(const vec3f& other);
    vec3f operator+(const vec3f& other) const;

    vec3f& operator*=(const vec3f& other);
    vec3f operator*(const vec3f& other) const;
    float dot(const vec3f& other) const;
    vec3f cross(const vec3f& other) const;

    vec3f& operator-=(const vec3f& other);
    vec3f operator-(const vec3f& other) const;
    vec3f operator-() const;

    float norm() const;

    vec3f& normalize();
    vec3f normalized() const;
    vec3f unit() const;

    vec3f& clamp(float low, float high);
    vec3f clamped(float low, float high) const;

    std::string to_string() const;
};

vec3f operator*(float u, const vec3f& v);

struct vec4f
{
    float x, y, z, w;

    vec4f& operator*=(float u);
    vec4f operator*(float u) const;

    vec4f& operator+=(const vec4f& other);
    vec4f operator+(const vec4f& other) const;

    vec4f operator-(const vec4f& other) const;

    vec3f to_cartesian() const;

    std::string to_string() const;
};

class mat_sq4f
{
    std::array<std::array<float, 4>, 4> rows;

public:
    // REVIEW: consider: std::array<float, 16> repr.; inline multiplication (no loops)
    // REVIEW: a better way to construct this beast
    mat_sq4f(std::array<std::array<float, 4>, 4> rows) : rows(rows) { }
    mat_sq4f(float a, float b, float c, float d,
             float e, float f, float g, float h,
             float i, float j, float k, float l,
             float m, float n, float o, float p) : rows{ a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p } { }

     float at(int i_row, int j_col) const;

    static mat_sq4f identity();

    mat_sq4f operator*(const mat_sq4f& other) const;

    vec3f operator*(const vec3f& v) const;
    vec4f mul_homo(const vec3f& v) const;

    vec3f operator*(const vec4f& v) const;

    std::vector<vec3f> operator*(const std::vector<vec3f>& vs) const;
    std::vector<vec4f> mul_homo(const std::vector<vec3f>& vs) const;

    std::vector<vec3f> operator*(const std::vector<vec4f>& vs) const;

private:
    void multiply_row(int i_row, float x);
    void subtract_rows(int i_row, int j_row, float x);

public:
    mat_sq4f transpose() const;
    mat_sq4f inverse() const;

    std::string to_string() const;
};

#endif //DZ_GAIKA_MATRICES_HPP
