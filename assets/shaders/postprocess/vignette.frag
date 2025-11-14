#version 330

// The texture holding the scene pixels
uniform sampler2D tex;

// Read "assets/shaders/fullscreen.vert" to know what "tex_coord" holds;
in vec2 tex_coord;

out vec4 frag_color;

// Vignette is a postprocessing effect that darkens the corners of the screen
// to grab the attention of the viewer towards the center of the screen

void main(){
    // Calculate the normalized device coordinates (NDC) ranging from -1 to 1
    vec2 ndc = tex_coord * 2.0 - 1.0;
    // Calculate the vignette factor based on the distance from the center
    float vignetteFactor = 1.0 + dot(ndc, ndc);
    // Apply the vignette effect by dividing the texture color by the vignette factor
    frag_color = texture(tex, tex_coord) / vignetteFactor;
}