#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in mat4 instanceMatrix;  // Per-instance model matrix (uses locations 5-8)

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 world_position;
    vec3 tangent;
} vs_out;

uniform mat4 VP;  // View-Projection matrix (no M since we use instanceMatrix)

void main(){
    // sending world position transformed by the instance matrix for calculations in frag shader
    vs_out.world_position = (instanceMatrix * vec4(position, 1.0)).xyz;
    
    // Transforming the normals using Inverse transpose to ensure they are not messed up
    // For instanced rendering, we compute the normal matrix from the instance matrix
    mat3 M_IT = transpose(inverse(mat3(instanceMatrix)));
    vs_out.normal = normalize(M_IT * normal);
    // Transform tangent by instanceMatrix (not M_IT) - tangents are direction vectors, not surface normals
    vs_out.tangent = normalize(mat3(instanceMatrix) * tangent);
    
    // Texture and color data for processing in frag shader
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    
    // Position after all transformations (VP * instanceMatrix * position)
    gl_Position = VP * instanceMatrix * vec4(position, 1.0);
}