
#ifndef DZ_GAIKA_MATRICES_HPP
#define DZ_GAIKA_MATRICES_HPP

#include <array>
#include <ostream>

class vec2s
{
    int16_t _x, _y;

public:
    vec2s(int16_t x, int16_t y) : _x(x), _y(y) { }

    int16_t x() const { return _x; }
    int16_t y() const { return _y; }
};

class vec3f
{
    std::array<float, 3> _xs;

public:
//    vec3f(std::array<float, 3> xs) : _xs(xs) { }
    vec3f(float x, float y, float z) : _xs{x, y, z} { }
    vec3f() = default;

    const std::array<float, 3>& xs() const { return _xs; };
    float at(size_t i) const { return _xs[i]; }
    float x() const { return _xs[0]; }
    float y() const { return _xs[1]; }
    float z() const { return _xs[2]; }

    vec3f operator*(float x) const;

    vec3f& operator+=(const vec3f& other);
    vec3f operator+(const vec3f& other) const;

    vec3f& operator*=(const vec3f& other);
    vec3f operator*(const vec3f& other) const;

    vec3f& operator-=(const vec3f& other);
    vec3f operator-(const vec3f& other) const;
    vec3f operator-() const;

    float norm() const;
    vec3f unit() const;

    // TODO: now, how do I call this and how should it be implemented?
    vec2s onto_xy_screen(int16_t x_size, int16_t y_size) const;

    friend std::ostream& operator<<(std::ostream& os, const vec3f& v);
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

    mat_sq4f operator*(const mat_sq4f& other) const;
    vec3f operator*(const vec3f& v) const;
    std::vector<vec3f> operator*(const std::vector<vec3f>& vs) const;

    friend std::ostream& operator<<(std::ostream& os, const mat_sq4f& m);
};

#endif //DZ_GAIKA_MATRICES_HPP
