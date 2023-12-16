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



void Render3DStuff()
{
    vector<Triangle> triangles = {
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

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glInterleavedArrays(GL_C3F_V3F, 0, triangles.data());
    glDrawArrays(GL_TRIANGLES, 0, triangles.size()*3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_DEPTH_TEST);
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
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("hopefully you can read this", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext opengl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, opengl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

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
        Render3DStuff();
        SDL_GL_SwapWindow(window);
    }

    ImGui::DestroyContext(ImGui::GetCurrentContext());
    SDL_Quit();

    return 0;
}