#version 330

// Uniforms for translation and scaling
uniform vec2 translation = vec2(0.0, 0.0);
uniform vec2 scale = vec2(1.0, 1.0);

out Varyings {
    vec3 color;
} vs_out;

void main(){
    const vec3 positions[3] = vec3[](
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );

    const vec3 colors[3] = vec3[](
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    vec3 position = positions[gl_VertexID];

    // Scale and translate the position
    position.xy = position.xy * scale + translation;
    
    // Send the transformed position to the pipeline
    gl_Position = vec4(position, 1.0);

    // Send the color to the fragment shader
    vs_out.color = colors[gl_VertexID];
}