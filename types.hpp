
#include <array>
//the obligatory 3d point class
class Point3D
{
    public:
    float x, y, z;
};
class RGBColor
{
    public:
    float red, green, blue;
};
class Vertex3D
{
    public:
    RGBColor color;
    Point3D pos;
};
class Triangle
{
    public:
    std::array<Vertex3D, 3> points;
};