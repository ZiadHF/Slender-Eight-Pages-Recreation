#pragma once

#include "mesh.hpp"
#include <string>

namespace our::mesh_utils {
    // Load an ".obj" file into the mesh
    Mesh* loadOBJ(const std::string& filename);
    
    // Load an ".obj" file with multiple materials support
    Mesh* loadOBJWithMaterials(const std::string& filename,bool keepCPUCopy);
    
    // Create a sphere (the vertex order in the triangles are CCW from the outside)
    // Segments define the number of divisions on the both the latitude and the longitude
    Mesh* sphere(const glm::ivec2& segments);
}