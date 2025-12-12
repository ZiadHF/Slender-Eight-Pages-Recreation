#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world_position;
} vs_out;

uniform mat4 transform;

void main(){
    vec4 world_pos = vec4(position, 1.0);
    gl_Position = transform * world_pos;
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    vs_out.world_position = world_pos.xyz;
}
