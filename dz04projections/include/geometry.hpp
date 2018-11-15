
#ifndef DZ04PROJECTIONS_GEOMETRY_HPP
#define DZ04PROJECTIONS_GEOMETRY_HPP

#include <vector>
#include <string>
#include <ostream>
#include <iomanip>

struct point3d
{
    const double x, y, z;

    std::string to_string() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

std::ostream& operator<<(std::ostream& os, const point3d& p)
{
    return os << "(" << std::setprecision(2) << p.x << "," << p.y << "," << p.z << ")";
}

struct triangle3d
{
    const point3d a, b, c;

    std::string to_string() const
    {
        return R"(▲{)" + a.to_string() + ", " + b.to_string() + ", " + c.to_string() + "}";
    }
};

std::ostream& operator<<(std::ostream& os, const triangle3d& tri)
{
    return os << R"(▲{)" << tri.a << ";" << tri.b << ";" << tri.c << "}";
}

struct shape3d
{
    const std::vector<triangle3d> faces;

    static shape3d tetrahedron(point3d a, point3d b, point3d c, point3d p)
    {
        return shape3d {
                {{ a, b, c }, { a, b, p }, { b, c, p }, { a, c, p }}
        };
    }
};

std::ostream& operator<<(std::ostream& os, const shape3d& shape)
{
    return os << "Shape{" << shape.faces.size() << " faces}";
}

#endif //DZ04PROJECTIONS_GEOMETRY_HPP
