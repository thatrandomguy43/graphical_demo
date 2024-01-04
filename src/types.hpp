#include <string>
#include <array>
#include <vector>
#include <memory>
#include "../sdl/include/SDL_opengl.h"
#include "../sdl/include/SDL.h"
#include "../glm/ext.hpp"
class ShaderFile
{
    public:
    std::string filename;
    GLenum type;
};
enum TextureID
{
    TEXTURE_TESTGRID = 1,
};


class Triangle
{
    public:
    std::array<glm::vec3, 3> points = {};
};
class Object3D
{
    public:
    std::vector<Triangle> mesh;
    glm::vec3 pos = {0, 0, 0};
    glm::vec3 axis = {0, 0, 1};
    float angle = 0;
    glm::vec3 scale = {1, 1, 1};
    TextureID texture;

};
//this is such a javaism but the standard library has forced my hand
class SDLSurfaceDeleter
{
    public:
    void operator()(SDL_Surface* p)
    {
        SDL_FreeSurface(p);
    }
};


