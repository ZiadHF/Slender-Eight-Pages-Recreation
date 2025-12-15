#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world_position;
} fs_in;

out vec4 frag_color;

uniform vec4 tint;
uniform sampler2D tex;

// Fog properties
uniform vec3 fog_color = vec3(0.0, 0.0, 0.0);  // Pitch black
uniform float horizon_threshold = 0.3;  // Y threshold for steep upward viewing
uniform bool fog_enabled = true;

void main(){
    vec4 result = tint * fs_in.color * texture(tex, fs_in.tex_coord);
    
    if (fog_enabled) {
        // Use the sphere's local Y coordinate to determine if we're at horizon level
        // For a sphere centered at origin with radius ~1: Y ranges from -1 (straight down) to +1 (straight up)
        // We want fog everywhere except when looking steeply upward
        float y_position = fs_in.world_position.y;
        
        // Apply fog to most of the sphere, only clear sky when looking steeply upward
        // Below threshold (0.7): fog_factor = 1.0 (full fog/pitch black)
        // Above threshold + 0.3: fog_factor = 0.0 (no fog, sky visible)
        float fog_factor = 1.0 - smoothstep(horizon_threshold, horizon_threshold + 0.3, y_position);
        
        // Mix with pitch black fog
        result.rgb = mix(result.rgb, fog_color, fog_factor);
    }
    
    frag_color = result;
}
