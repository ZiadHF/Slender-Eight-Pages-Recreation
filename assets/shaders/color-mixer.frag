#version 330 core

// Uniforms for channel mixing
uniform vec4 red = vec4(1.0, 0.0, 0.0, 0.0);
uniform vec4 green = vec4(0.0, 1.0, 0.0, 0.0);
uniform vec4 blue = vec4(0.0, 0.0, 1.0, 0.0);

in Varyings {
    vec3 color;
} fs_in;

out vec4 frag_color;

void main(){
    frag_color.r = dot(red.rgb, fs_in.color) + red.a;
    frag_color.g = dot(green.rgb, fs_in.color) + green.a;
    frag_color.b = dot(blue.rgb, fs_in.color) + blue.a;
    frag_color.a = 1.0;
}