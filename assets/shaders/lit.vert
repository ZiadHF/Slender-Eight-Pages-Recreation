#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 world_position;
} vs_out;

uniform mat4 transform;
uniform mat4 M;
uniform mat4 M_IT;

void main(){
    // sending world position transformed by the model matrix for calculations in frag shader
    vs_out.world_position = (M * vec4(position,1)).xyz;
    // Transforming the normals using Inverse transpose to ensure they are not messed up
    vs_out.normal = normalize((M_IT * vec4(normal,0)).xyz);
    // Texture and color data for processing in frag shader
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    // Position after all transformations
    gl_Position = transform * vec4(position, 1.0);
}