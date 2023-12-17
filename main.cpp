#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "sdl/include/SDL.h"
#include "sdl/include/SDL_opengl.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "types.hpp"
using namespace std;

ofstream LOG;

namespace GL {
    //a VAO vertex array object stores metadata about what array each vertex attribute index (not really an array index, just an arbitrary number chosen by you) is associated with and what that array's data format is
    //all the data is stored in the GL_ARRAY_BUFFER, so you use the metadata to interleave the multiple attribute arrays, which also seems to be required because you can't tell it when to stop reading one atttrib and start reading another
    //since each vertex is supposed ot have one of each attribute, the interleaved arrays must be of equal length it seems 

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
}


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
}

void RenderMesh(const vector<Triangle>& mesh)
{


    glEnable(GL_DEPTH_TEST);
    println(LOG, "glEnable error number {}", glGetError());
    glInterleavedArrays(GL_C3F_V3F, 0, mesh.data());
    println(LOG, "glInterleavedArrays error number {}", glGetError());
    glDrawArrays(GL_TRIANGLES, 0, mesh.size()*3);
    println(LOG, "glDrawArrays error number {}", glGetError());
    glDisableClientState(GL_COLOR_ARRAY);
    println(LOG, "glDisableClientState error number {}", glGetError());
    glDisableClientState(GL_VERTEX_ARRAY);
    println(LOG, "glDisableClientState error number {}", glGetError());
    glDisable(GL_DEPTH_TEST);
    println(LOG, "glDisable error number {}", glGetError());
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

    LoadOpenGL3Funcs();

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