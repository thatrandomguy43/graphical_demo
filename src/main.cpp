#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../sdl/include/SDL.h"
#include "../sdl/include/SDL_opengl.h"
#include "../glm/ext.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "types.hpp"
using namespace std;

ofstream LOG;

namespace GL {
    //a VAO vertex array object stores metadata about what array each vertex attribute index is associated with and what that array's data format is
    //indexes can be chosen arbitrarily between 0 and GL_MAX_VERTEX_ATTRIBS - 1, but you need to bind a name to them to use them in shaders
    //no worries about that rn tho

    //the actual data is stored inside a buffer you bind to GL_ARRAY_BUFFER, but you use GL::VertexAttribPointer to define multiple either interleaved or seperated arrays
    GLenum (*GetError)() = nullptr;
    void (*Enable)(GLenum cap) = nullptr;
    void (*Disable)(GLenum cap) = nullptr;
    void (*Clear)(GLbitfield mask) = nullptr;
    void (*Viewport)(GLint x, GLint y, GLsizei width, GLsizei height) = nullptr;
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
    void (*GetShaderiv)(GLuint shader, GLenum pname, GLint *params) = nullptr;
    void (*GetShaderInfoLog)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) = nullptr;
    void (*GetProgramiv)(GLuint shader, GLenum pname, GLint *params) = nullptr;
    void (*GetProgramInfoLog)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) = nullptr;
    void (*LinkProgram)(GLuint program) = nullptr;
    void (*UseProgram)(GLuint program) = nullptr;
    GLint (*GetUniformLocation)(GLuint program, const GLchar* name) = nullptr;
    void (*UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = nullptr;
}

GLuint VAO_HANDLE = 0;
GLuint VERTEX_BUFFER_HANDLE = 0;
GLuint RENDER_PROGRAM_HANDLE = 0;
const GLuint POSITION_ATTRIB_INDEX = 0;
const GLuint COLOR_ATTRIB_INDEX = 1;
GLuint MODEL_UNIFORM_INDEX;
GLuint VIEW_UNIFORM_INDEX;
GLuint PROJECTION_UNIFORM_INDEX;

class CubeOptions 
{
    public:
    float angle;
    glm::vec3 pos;
    glm::vec3 axis;
    glm::vec3 scale;
};

CubeOptions LayoutUI()
{
    static bool show_demo = false;
    if (ImGui::IsKeyPressed(ImGuiKey_D))
    {
        show_demo = !show_demo;
    }
    if (show_demo) {
        ImGui::ShowDemoWindow();
    } 

    ImGui::Begin("cube options");
    CubeOptions opt;
    static bool first_frame = true;
    if (first_frame)
    {
        first_frame = false;
        opt.angle = 0;
        opt.axis = {0, 1, 0};
        opt.scale = {1, 1, 1};
        opt.pos = {0, 0, -3};
    }
    ImGui::SliderFloat3("Position", &opt.pos.x, -5, 5);
    ImGui::SliderFloat3("Scale", &opt.scale.x, 0, 4);
    ImGui::SliderFloat3("Axis of rotation", &opt.axis.x, 0, 1);
    ImGui::SliderAngle("Angle", &opt.angle, 0, 360);
    ImGui::End();
    return opt;
}
//i hate that i have to write this code but like 6 layers of nested initializers is impossible to write
vector<Triangle> GenCube()
{
    const vector<glm::vec4> colors = {
        {1, 0, 0, 1}, {1, 0, 0, 1},
        {1, 1, 0, 1}, {1, 1, 0, 1},
        {1, 0, 1, 1}, {1, 0, 1, 1},
        {0, 1, 0, 1}, {0, 1, 0, 1},
        {0, 1, 1, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {0, 0, 1, 1}
        };
    vector<Triangle> cube = {
        {Vertex3D{{-1, -1, -1}}, {{-1, 1, -1}}, {{-1, 1, 1}}}, 
        {Vertex3D{{-1, -1, -1}}, {{-1, -1, 1}}, {{-1, 1, 1}}}
    };
    cube.resize(12);

    for (int i = 2; i < cube.size(); i += 2)
    {
        cube[i] = cube[0];
        cube[i+1] = cube[1];
    }
    for (int i = 0; i < cube.size(); i++)
    {
        for (Vertex3D& pt : cube[i].points)
        {
            pt.color = colors[i];
        } 
        if (i % 4 == 2 or i % 4 == 3) 
        {
            for (Vertex3D& pt : cube[i].points)
            {
                pt.pos.x = 1;
            } 
        }
        if (i >= 4 and i < 8) 
        {
            for (Vertex3D& pt : cube[i].points)
            {
                pt.pos = {pt.pos.y, pt.pos.z, pt.pos.x};
            } 
        }
        else if (i >= 8)
        {
            for (Vertex3D& pt : cube[i].points)
            {
                pt.pos = {pt.pos.z, pt.pos.x, pt.pos.y};
            } 
        }
    }
    return cube;
}


void LoadOpenGLFuncs()
{
    GL::GetError = (GLenum (*)())SDL_GL_GetProcAddress("glGetError");
    GL::Enable = (void (*)(GLenum))SDL_GL_GetProcAddress("glEnable");
    GL::Disable = (void (*)(GLenum))SDL_GL_GetProcAddress("glDisable");
    GL::Clear = (void (*)(GLbitfield))SDL_GL_GetProcAddress("glClear");
    GL::Viewport = (void (*)(GLint, GLint, GLsizei, GLsizei))SDL_GL_GetProcAddress("glViewport");
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
    GL::GetShaderiv = (void (*)(GLuint, GLenum, GLint*))SDL_GL_GetProcAddress("glGetShaderiv");
    GL::GetShaderInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*))SDL_GL_GetProcAddress("glGetShaderInfoLog");
    GL::GetProgramiv = (void (*)(GLuint, GLenum, GLint*))SDL_GL_GetProcAddress("glGetProgramiv");
    GL::GetProgramInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*))SDL_GL_GetProcAddress("glGetProgramInfoLog");
    GL::LinkProgram = (void (*)(GLuint))SDL_GL_GetProcAddress("glLinkProgram");
    GL::UseProgram = (void (*)(GLuint))SDL_GL_GetProcAddress("glUseProgram");
    GL::GetUniformLocation = (GLint (*)(GLuint, const GLchar*))SDL_GL_GetProcAddress("glGetUniformLocation");
    GL::UniformMatrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*))SDL_GL_GetProcAddress("glUniformMatrix4fv");

}

string FileToString(const string& filename)
{
    ifstream file;
    string out;
    file.open(filename);
    while (not file.eof() and not file.fail()) 
    {
        char got;
        file.get(got);
        out.append(string{got});
    }
    return out;
}

void InitOpenGLFor3D(vector<ShaderFile> shaders_to_load)
{
    LoadOpenGLFuncs();

    RENDER_PROGRAM_HANDLE = GL::CreateProgram();

    vector<GLuint> shader_handles;
    for (auto& shader : shaders_to_load)
    {
        string source = FileToString(shader.filename);
        source.push_back('\n');
        const char* source_c_str = source.c_str();
        GLuint current_shader_handle = GL::CreateShader(shader.type);
        shader_handles.push_back(current_shader_handle);
        GL::ShaderSource(current_shader_handle, 1, &source_c_str, nullptr);
        GL::CompileShader(current_shader_handle);
        string compile_log;
        GLint log_size;
        GL::GetShaderiv(current_shader_handle, GL_INFO_LOG_LENGTH, &log_size);
        compile_log.resize(log_size);
        GL::GetShaderInfoLog(current_shader_handle, log_size, nullptr, compile_log.data());
        if (not compile_log.empty() and compile_log.back() == '\0') 
        {
            compile_log.pop_back();
        }
        println(LOG, "compile log for {}: {}", shader.filename, compile_log);
        GL::AttachShader(RENDER_PROGRAM_HANDLE, current_shader_handle);
    }
    GL::LinkProgram(RENDER_PROGRAM_HANDLE);
    string link_log;
    GLint log_size;
    GL::GetProgramiv(RENDER_PROGRAM_HANDLE, GL_INFO_LOG_LENGTH, &log_size);
    link_log.resize(log_size);
    GL::GetProgramInfoLog(RENDER_PROGRAM_HANDLE, log_size, nullptr, link_log.data());
    if (not link_log.empty() and link_log.back() == '\0') 
    {
        link_log.pop_back();
    }
    println(LOG, "link log for render program: {}", link_log);
    GL::UseProgram(RENDER_PROGRAM_HANDLE);
    MODEL_UNIFORM_INDEX = GL::GetUniformLocation(RENDER_PROGRAM_HANDLE, "MODEL");
    VIEW_UNIFORM_INDEX = GL::GetUniformLocation(RENDER_PROGRAM_HANDLE, "VIEW");
    PROJECTION_UNIFORM_INDEX = GL::GetUniformLocation(RENDER_PROGRAM_HANDLE, "PROJECTION");
    GL::GenVertexArrays(1, &VAO_HANDLE);
    GL::BindVertexArray(VAO_HANDLE);
    GL::GenBuffers(1, &VERTEX_BUFFER_HANDLE);
    GL::BindBuffer(GL_ARRAY_BUFFER, VERTEX_BUFFER_HANDLE);
    GL::VertexAttribPointer(POSITION_ATTRIB_INDEX, 3, GL_FLOAT, true, sizeof(Vertex3D), 0);
    GL::VertexAttribPointer(COLOR_ATTRIB_INDEX, 4, GL_FLOAT, true, sizeof(Vertex3D), (void*)sizeof(glm::vec3));
    GL::EnableVertexAttribArray(POSITION_ATTRIB_INDEX);
    GL::EnableVertexAttribArray(COLOR_ATTRIB_INDEX);
    
}


void RenderMesh(const vector<Triangle>& mesh)
{
    GL::BufferData(GL_ARRAY_BUFFER, mesh.size()*sizeof(Triangle), mesh.data(), GL_STREAM_DRAW);
    GL::Enable(GL_DEPTH_TEST);
    GL::DrawArrays(GL_TRIANGLES, 0, mesh.size()*3);
    GL::Disable(GL_DEPTH_TEST);
}

void RenderObjects(const vector<Object3D>& objects, const glm::mat4& view, const glm::mat4& perspective)
{
    GL::UniformMatrix4fv(VIEW_UNIFORM_INDEX, 1, false, &view[0][0]);
    GL::UniformMatrix4fv(PROJECTION_UNIFORM_INDEX, 1, false, &perspective[0][0]);
    
    for (const Object3D& obj : objects)
    {
        //whoever decided that transformations should put the input on the LEFT, thus requiring the transformations in the OPPOSITE ORDER THAN WHAT EVERYONE SAYS TO DO can go fuck themselves
        glm::mat4 model = glm::identity<glm::mat4>();
        model = glm::translate(model, obj.pos);
        model = glm::scale(model, obj.scale);
        model = glm::rotate(model, obj.angle, obj.axis);
        GL::UniformMatrix4fv(MODEL_UNIFORM_INDEX, 1, false, &model[0][0]);
        RenderMesh(obj.mesh);
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("hopefully you can read this", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext opengl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, opengl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync


    InitOpenGLFor3D({{"src/mvp.vert.glsl", GL_VERTEX_SHADER}, {"src/noop.frag.glsl", GL_FRAGMENT_SHADER}});

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
        

        CubeOptions options = LayoutUI();
        ImGui::Render();
        int window_width, window_height;
        SDL_GL_GetDrawableSize(window, &window_width, &window_height);
        GL::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL::Viewport(0, 0, window_width, window_height);
        Object3D cube = {Object3D{GenCube(), options.pos, options.axis, options.angle, options.scale}};
        glm::mat4 view = glm::lookAt<float>(glm::vec3{0, 0, 0}, glm::vec3{0, 0, -1}, glm::vec3{0.0, 1.0, 0.0});
        glm::mat4 perspective = glm::perspective<float>(glm::radians<float>(70), static_cast<float>(window_width)/static_cast<float>(window_height), 0.25, 100);
        RenderObjects({cube}, view, perspective);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui::DestroyContext(ImGui::GetCurrentContext());
    SDL_Quit();

    return 0;
}