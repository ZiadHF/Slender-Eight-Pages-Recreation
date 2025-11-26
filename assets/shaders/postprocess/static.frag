#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

// Uniforms for controlling the static effect intensity
// distance: distance between player and slenderman (large = far = no static)
// angle: angle between player's view direction and slenderman's position (large = not looking = no static)
// time: how long player has been looking at slenderman
uniform float distance = 999.0;  // Default: far away (no static)
uniform float angle = 180.0;     // Default: not looking (no static)  
uniform float time = 0.0;        // Default: not looking (no static)

// Random noise function
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main(){
    vec4 originalColor = texture(tex, tex_coord);

    // Calculate intensity based on distance, angle, and time
    float distanceFactor = clamp(1.0 - (distance / 50.0), 0.0, 1.0);
    float angleFactor = clamp(1.0 - (angle / 1.57), 0.0, 1.0);
    float timeFactor = clamp(time / 5.0, 0.0, 1.0);
    float intensity = distanceFactor * angleFactor * (0.3 + 0.7 * timeFactor);

    // CRT TV Static with horizontal banding
    // Create horizontal bands that shift over time
    float bandY = floor(tex_coord.y * 120.0 + time * 30.0); // Horizontal band index
    float bandOffset = random(vec2(bandY, floor(time * 20.0))) * 2.0 - 1.0; // Random offset per band
    
    // Fine grain noise coordinates
    vec2 noiseCoord = tex_coord * vec2(512.0, 256.0);
    noiseCoord.x += bandOffset * 50.0; // Shift pixels horizontally per band
    noiseCoord.y += time * 500.0; // Vertical scroll
    
    // Generate base static noise
    float noise1 = random(floor(noiseCoord) + floor(time * 60.0));
    
    // Second layer at different frequency for texture
    vec2 noiseCoord2 = tex_coord * vec2(256.0, 128.0);
    noiseCoord2.x += bandOffset * 30.0;
    float noise2 = random(floor(noiseCoord2) + floor(time * 45.0));
    
    // Combine noise layers
    float staticNoise = mix(noise1, noise2, 0.4);
    
    // Add horizontal streak intensity variation per band
    float bandBrightness = random(vec2(bandY * 0.1, floor(time * 15.0)));
    bandBrightness = mix(0.7, 1.3, bandBrightness); // Vary brightness per band
    
    // Apply band brightness to create the streaky look
    staticNoise *= bandBrightness;
    
    // Clamp to valid range
    staticNoise = clamp(staticNoise, 0.0, 1.0);
    
    // Create the static color (grayscale snow with banding)
    vec3 staticColor = vec3(staticNoise);
    
    // Blend static over the original image based on intensity
    vec3 finalColor = mix(originalColor.rgb, staticColor, intensity);
    
    frag_color = vec4(finalColor, 1.0);
}