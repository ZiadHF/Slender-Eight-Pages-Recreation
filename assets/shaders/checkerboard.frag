#version 330 core

out vec4 frag_color;

// Uniforms for checkerboard size and colors
uniform int size = 32;
uniform vec3 colors[2];

void main(){
    ivec2 coord = ivec2(gl_FragCoord.xy) / size;
    frag_color = vec4(colors[(coord.x + coord.y) % 2], 1.0);
}