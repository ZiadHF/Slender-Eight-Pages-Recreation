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
        glm::vec3 posRandom;      // Random position offset
        glm::vec3 rotationRandom; // Random rotation offset
        float scaleRandom;        // Random scale multiplier
    };

    
    class TreePositionRegistry {
    public:
        static TreePositionRegistry& getInstance() {
            static TreePositionRegistry instance;
            return instance;
        }

        
        bool isPositionValid(const glm::vec3& pos, float minDistance) const {
            for (const auto& existing : positions) {
                float dist = glm::length(glm::vec2(pos.x - existing.x, pos.z - existing.z));
                if (dist < minDistance) {
                    return false;
                }
            }
            return true;
        }

        void registerPosition(const glm::vec3& pos) {
            positions.push_back(pos);
        }

        void clear() {
            positions.clear();
        }

    private:
        std::vector<glm::vec3> positions;
        TreePositionRegistry() = default;
    };

    std::vector<TreeInstance> generateFromMap(const std::string &mapFilename, glm::vec2 worldSize, float density,
                                              glm::vec3 posRandomRange = glm::vec3(3.0f),
                                              glm::vec3 rotRandomRange = glm::vec3(0.0f, 6.28318530718f, 0.0f),
                                              glm::vec2 scaleRandomRange = glm::vec2(0.0f, 0.5f),
                                              float minTreeDistance = 2.0f,
                                              bool usePositionRegistry = true);
}
