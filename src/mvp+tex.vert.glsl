#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
out vec4 vertex_color;

void main()
{
    gl_Position = PROJECTION * VIEW * MODEL * vec4(position, 1.0);
    vertex_color = color;
}
