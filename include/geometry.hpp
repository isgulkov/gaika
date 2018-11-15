
#ifndef DZ04PROJECTIONS_GEOMETRY_HPP
#define DZ04PROJECTIONS_GEOMETRY_HPP

#include <vector>
#include <string>
#include <array>
#include <ostream>
#include <iomanip>

template <typename T, std::size_t NDim>
class vec_t : public std::array<const T, NDim>
{
public:
    template <typename ...Fuck>
    vec_t(Fuck&&... f) : std::array<const T, NDim>{ std::forward<Fuck>(f)... } { }

    friend std::ostream& operator<<(std::ostream& os, const vec_t<T, NDim>& v);
};

template <typename T, size_t NDim>
std::ostream& operator<<(std::ostream& os, const vec_t<T, NDim>& v)
{
    os << "(";

    //

    return os << ")";
}

typedef vec_t<float, 3> vec3f;
typedef vec_t<int16_t, 2> vec2s;

template <typename Tv>
struct triangle
{
    const Tv a, b, c;
};

template <typename Tv>
std::ostream& operator<<(std::ostream& os, const triangle<Tv>& tri)
{
    return os << R"(▲{)" << tri.a << ";" << tri.b << ";" << tri.c << "}";
}

struct phedron3f
{
    const std::vector<triangle<vec3f>> faces;

    static phedron3f tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
    {
        return phedron3f {
                {{ a, b, c }, { a, b, p }, { b, c, p }, { a, c, p }}
        };
    }
};

std::ostream& operator<<(std::ostream& os, const phedron3f& solid)
{
    return os << "S{" << solid.faces.size() << "f}";
}

#endif //DZ04PROJECTIONS_GEOMETRY_HPP
