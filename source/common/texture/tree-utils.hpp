#pragma once

#include <string>

#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace our
{
    struct TreeInstance
    {
        glm::vec3 pos;
        float scale;
        float rotation; // Y-axis rotation in radians
        float widthScale; // Additional horizontal scaling (X and Z axes)
    };
    
    std::vector<TreeInstance> generateFromMap(const std::string &mapFilename, glm::vec2 worldSize, float density);
}
