#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;
layout(location = 4) in mat4 instanceMatrix;

out Varyings {
    vec4 color;
    vec2 tex_coord;
} vs_out;

uniform mat4 VP;

void main(){
    gl_Position = VP * instanceMatrix * vec4(position, 1.0);
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
}
