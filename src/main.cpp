#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../sdl/include/SDL.h"
#include "../sdl/include/SDL_opengl.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include "types.hpp"
using namespace std;

ofstream LOG;

namespace GL {
    //a VAO vertex array object stores metadata about what array each vertex attribute index is associated with and what that array's data format is
    //indexes can be chosen arbitrarily between 0 and GL_MAX_VERTEX_ATTRIBS - 1, but you need to bind a name to them to use them in shaders
    //no worries about that rn tho

    //the actual data is stored inside a buffer you bind to GL_ARRAY_BUFFER, but you use GL::VertexAttribPointer to define multiple either interleaved or seperated arrays

    void (*GenVertexArrays)(GLsizei n, GLuint *arrays) = nullptr; //create n VAO handle and store them in the given array (i should only ever need 1)
    void (*BindVertexArray)(GLuint array) = nullptr; //make the given VAO the currently active one
    void (*DeleteVertexArrays)(GLsizei n, const GLuint *arrays) = nullptr; //delete the n VAOs specified in the given array
    void (*GenBuffers)(GLsizei n, GLuint* buffers) = nullptr; //create buffer handle
    void (*BindBuffer)(GLenum target, GLuint buffer) = nullptr; //bind buffer handle to one of the built in render buffers (for me mostly GL_ARRAY_BUFFER), if buffer 0 is specified it just unbinds whatever was bound tho that builtin
    void (*BufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) = nullptr; //give the specified builtin buffer actual data storage, and also filling in that data
    void (*DeleteBuffers)(GLsizei n, const GLuint *arrays) = nullptr; //delete the n buffers specified in the given array
    void (*VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) = nullptr; //defines a new index with its metadata or modifies an existing index in the currently bound VAO
    void (*EnableVertexAttribArray)(GLuint index) = nullptr; //enable vertex arrib array on current VAO
    void (*DisableVertexAttribArray)(GLuint index) = nullptr; //disable vertex arrib array on current VAO
    void (*DrawArrays)(GLenum mode, GLint first, GLsizei count) = nullptr; //draws from the vertex attrib arrays enabled on the current VAO
    GLuint (*CreateProgram)() = nullptr;
    GLuint (*CreateShader)(GLenum shaderType) = nullptr;
    void (*DeleteShader)(GLuint shader) = nullptr;
    void (*DeleteProgram)(GLuint program) = nullptr;
    void (*ShaderSource)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) = nullptr;
    void (*AttachShader)(GLuint program, GLuint shader) = nullptr;
    void (*DetachShader)(GLuint program, GLuint shader) = nullptr;
    void (*CompileShader)(GLuint shader) = nullptr;
    void (*GetShaderInfoLog)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
    void (*LinkProgram)(GLuint program) = nullptr;
    void (*UseProgram)(GLuint program) = nullptr;

}

GLuint VAO_HANDLE = 0;
GLuint VERTEX_BUFFER_HANDLE = 0;
GLuint RENDER_PROGRAM_HANDLE = 0;
void LayoutUI()
{
    static bool show_demo = false;
    if (ImGui::IsKeyPressed(ImGuiKey_D))
    {
        show_demo = !show_demo;
    }
    if (show_demo) {
        ImGui::ShowDemoWindow();
    } 
    ImGui::Begin("epic ass window");
    ImGui::Button("press", {50, 50});
    if (ImGui::IsItemActive())
    {
        ImGui::Text("pressing");
    }
    else 
    {
        ImGui::Text("not pressing");
    }
    ImGui::End();

}

const vector<Triangle> test_triangles = {
    Triangle{
    Vertex3D{ RGBColor{1.0, 0.5, 0}, Point3D{-1.0,-1.0,0.0,},},
    Vertex3D{ RGBColor{1.0, 0.5, 0}, Point3D{-0.5,1.0,0.5,},},
    Vertex3D{ RGBColor{1.0, 0.5, 0}, Point3D{0.5,0.0,1.0,},}
    }, Triangle{
    Vertex3D{ RGBColor{1.0, 0, 0,}, Point3D{-0.5,0.5,0.75,},},
    Vertex3D{ RGBColor{0, 1.0, 0,}, Point3D{0.5,-0.5,0.75,},},
    Vertex3D{ RGBColor{0, 0, 1.0,}, Point3D{-0.5,-0.5, 0.75,},}
    }
};


void LoadOpenGL3Funcs()
{
    GL::GenVertexArrays = (void (*)(GLsizei, GLuint*))SDL_GL_GetProcAddress("glGenVertexArrays");
    GL::BindVertexArray = (void (*)(GLuint))SDL_GL_GetProcAddress("glBindVertexArray");
    GL::DeleteVertexArrays = (void (*)(GLsizei, const GLuint*))SDL_GL_GetProcAddress("glDeleteVertexArrays");
    GL::GenBuffers = (void (*)(GLsizei, GLuint*))SDL_GL_GetProcAddress("glGenBuffers");
    GL::BindBuffer = (void (*)(GLenum, GLuint))SDL_GL_GetProcAddress("glBindBuffer");
    GL::BufferData = (void (*)(GLenum, GLsizeiptr, const GLvoid*, GLenum))SDL_GL_GetProcAddress("glBufferData");
    GL::DeleteBuffers = (void (*)(GLsizei, const GLuint*))SDL_GL_GetProcAddress("glDeleteBuffers");
    GL::VertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*))SDL_GL_GetProcAddress("glVertexAttribPointer");
    GL::EnableVertexAttribArray = (void (*)(GLuint))SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    GL::DisableVertexAttribArray = (void (*)(GLuint))SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    GL::DrawArrays = (void (*)(GLenum, GLint, GLsizei))SDL_GL_GetProcAddress("glDrawArrays");
    GL::CreateProgram = (GLuint (*)())SDL_GL_GetProcAddress("glCreateProgram");
    GL::CreateShader = (GLuint (*)(GLenum))SDL_GL_GetProcAddress("glCreateShader");
    GL::DeleteShader = (void (*)(GLuint))SDL_GL_GetProcAddress("glDeleteShader");
    GL::DeleteProgram = (void (*)(GLuint))SDL_GL_GetProcAddress("glDeleteProgram");
    GL::ShaderSource = (void (*)(GLuint, GLsizei, const GLchar**, const GLint*))SDL_GL_GetProcAddress("glShaderSource");
    GL::AttachShader = (void (*)(GLuint, GLuint))SDL_GL_GetProcAddress("glAttachShader");
    GL::DetachShader = (void (*)(GLuint, GLuint))SDL_GL_GetProcAddress("glDetachShader");
    GL::CompileShader = (void (*)(GLuint))SDL_GL_GetProcAddress("glCompileShader");
    GL::LinkProgram = (void (*)(GLuint))SDL_GL_GetProcAddress("glLinkProgram");
    GL::UseProgram = (void (*)(GLuint))SDL_GL_GetProcAddress("glUseProgram");
}

vector<string> FileToStringArray(const string& filename)
{
    ifstream file;
    vector<string> out;
    file.open(filename);
    while (not file.eof() and not file.fail()) 
    {
        out.push_back("");
        char got = '\0';
        while (got != '\n' and not file.eof()) 
        {
            file.get(got);
            if (got != '\n')
            {
                out.back().append(string{got});
            }
            
        }
    }
    return out;
}

void InitOpenGLFor3D()
{
    LoadOpenGL3Funcs();

    RENDER_PROGRAM_HANDLE = GL::CreateProgram();

    vector<vector<string>> shader_sources = {FileToStringArray("./src/noop.vertex.glsl"), FileToStringArray("./src/noop.geometry.glsl"), FileToStringArray("./src/noop.fragment.glsl")};
    vector<GLuint> shader_handles = {GL::CreateShader(GL_VERTEX_SHADER), GL::CreateShader(GL_GEOMETRY_SHADER), GL::CreateShader(GL_FRAGMENT_SHADER)};
    for (size_t shader = 0; shader < shader_sources.size() and shader < shader_handles.size(); shader++)
    {
    vector<GLint> code_line_sizes;
    vector<const char*> code_line_c_strs;
    for (auto line : shader_sources[shader])
    {
        code_line_sizes.push_back(line.size());
        code_line_c_strs.push_back(line.c_str());
    }
    GL::ShaderSource(shader_handles[shader], shader_sources[shader].size(), code_line_c_strs.data(), code_line_sizes.data());
    }



}

void RenderMesh(const vector<Triangle>& mesh)
{
    static bool has_initted_buffer = false;

    if (not has_initted_buffer)
    {
        GL::GenVertexArrays(1, &VAO_HANDLE);
        GL::BindVertexArray(VAO_HANDLE);
        GL::GenBuffers(1, &VERTEX_BUFFER_HANDLE);
        GL::BindBuffer(GL_ARRAY_BUFFER, VERTEX_BUFFER_HANDLE);
        has_initted_buffer = true;
    }

}

int main(int argc, char* argv[])
{
    LOG.open("./guiapp_log.txt", ios::out);
    int err = SDL_Init(SDL_INIT_EVERYTHING);
    //go reference!
    if (err != 0) 
    {
        println(LOG, "damn bruh sdl couldn't start, this the error: {}", SDL_GetError());
        return 1;
    }
    //i have no idea what this code does, well i guess i kinda know
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("hopefully you can read this", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext opengl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, opengl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    int minor, major;


    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    InitOpenGLFor3D();

    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, opengl_context);
    ImGui_ImplOpenGL3_Init();


    while (true)
    {
        SDL_Event event;
        
        if (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_WINDOWEVENT and event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                break;
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        

        LayoutUI();
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 1280, 720);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        RenderMesh(test_triangles);
        SDL_GL_SwapWindow(window);
    }

    ImGui::DestroyContext(ImGui::GetCurrentContext());
    SDL_Quit();

    return 0;
}