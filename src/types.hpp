#include <string>
#include <array>
#include <vector>
#include "../sdl/include/SDL_opengl.h"
#include "../glm/ext.hpp"
//the obligatory 3d point class
class ShaderFile
{
    public:
    std::string filename;
    GLenum type;
};
class Vertex3D
{
    public:
    glm::vec3 pos;
    glm::vec4 color;
};
class Triangle
{
    public:
    std::array<Vertex3D, 3> points;
};
class Object3D
{
    public:
    std::vector<Triangle> mesh;
    glm::vec3 pos = {0, 0, 0};
    glm::vec3 axis = {0, 0, 1};
    float angle = 0;
    glm::vec3 scale = {1, 1, 1};

};