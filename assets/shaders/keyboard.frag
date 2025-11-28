#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
} fs_in;

out vec4 frag_color;

uniform vec4 tint;
uniform sampler2D tex;
uniform bool highlighted = false;

void main(){
    vec4 tex_color = texture(tex, fs_in.tex_coord);
    
    if (highlighted) {
        // Target the gray color of the key
        vec3 target_gray = vec3(88.0/255.0, 88.0/255.0, 88.0/255.0);
        
        // Highlight color (golden yellow)
        vec3 highlight_color = vec3(255.0/255.0, 216.0/255.0, 56.0/255.0);
        
        // Check if the pixel is close to the target gray color
        float threshold = 1;
        float dist = distance(tex_color.rgb, target_gray);
        float blend = 1.0 - (dist / threshold);
        tex_color.rgb = mix(tex_color.rgb, highlight_color, blend);
     
        float brightness = 1.2; // Slight brightness boost
        tex_color.rgb *= brightness;
    }
    
    frag_color = tint * fs_in.color * tex_color;
}