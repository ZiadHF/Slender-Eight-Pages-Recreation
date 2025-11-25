#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../ecs/component.hpp"
#include "../material/material.hpp"
#include "../mesh/mesh.hpp"

namespace our {

// Frustum planes for culling
struct Frustum {
    glm::vec4 planes[6];  // left, right, bottom, top, near, far

    void extractFromVP(const glm::mat4& VP) {
        // Extract frustum planes from view-projection matrix
        // Left plane
        planes[0] = glm::vec4(VP[0][3] + VP[0][0], VP[1][3] + VP[1][0],
                              VP[2][3] + VP[2][0], VP[3][3] + VP[3][0]);
        // Right plane
        planes[1] = glm::vec4(VP[0][3] - VP[0][0], VP[1][3] - VP[1][0],
                              VP[2][3] - VP[2][0], VP[3][3] - VP[3][0]);
        // Bottom plane
        planes[2] = glm::vec4(VP[0][3] + VP[0][1], VP[1][3] + VP[1][1],
                              VP[2][3] + VP[2][1], VP[3][3] + VP[3][1]);
        // Top plane
        planes[3] = glm::vec4(VP[0][3] - VP[0][1], VP[1][3] - VP[1][1],
                              VP[2][3] - VP[2][1], VP[3][3] - VP[3][1]);
        // Near plane
        planes[4] = glm::vec4(VP[0][3] + VP[0][2], VP[1][3] + VP[1][2],
                              VP[2][3] + VP[2][2], VP[3][3] + VP[3][2]);
        // Far plane
        planes[5] = glm::vec4(VP[0][3] - VP[0][2], VP[1][3] - VP[1][2],
                              VP[2][3] - VP[2][2], VP[3][3] - VP[3][2]);
        // Normalize planes
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(planes[i]));
            planes[i] /= length;
        }
    }

    // Check if a sphere is inside or intersects the frustum
    bool isSphereInside(const glm::vec3& center, float radius) const {
        for (int i = 0; i < 6; i++) {
            float distance =
                glm::dot(glm::vec3(planes[i]), center) + planes[i].w;
            if (distance < -radius) {
                return false;  // Sphere is completely outside this plane
            }
        }
        return true;
    }
};

class InstancedRendererComponent : public Component {
   public:
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    std::vector<glm::mat4> InstanceMats;       // All instance matrices
    std::vector<glm::vec3> instancePositions;  // Cached positions for culling
    std::vector<glm::mat4>
        visibleInstanceMats;  // Visible instances after culling
    std::unordered_map<std::string, Material*> submeshMaterials;

    // Culling settings
    float maxRenderDistance = 100.0f;  // Maximum distance to render instances
    float cullingBoundingRadius =
        10.0f;  // Bounding sphere radius for frustum culling
    bool enableDistanceCulling = true;
    bool enableFrustumCulling = true;

    // Configuration for instance transforms
    glm::vec3 scaleMultiplier = glm::vec3(1.0f);
    glm::vec2 scaleRandomRange =
        glm::vec2(0.0f);  // min, max offset from multiplier

    glm::vec3 rotationMultiplier = glm::vec3(0.0f);
    glm::vec3 rotationRandomRange = glm::vec3(0.0f);

    glm::vec3 positionOffset = glm::vec3(0.0f);
    glm::vec3 positionRandomRange = glm::vec3(0.0f);

    static std::string getID() { return "Instanced Renderer"; }

    Material* getMaterialForSubmesh(const std::string& submeshName) const {
        auto it = submeshMaterials.find(submeshName);
        return (it != submeshMaterials.end()) ? it->second : material;
    }

    // Perform culling and update visible instances
    void updateVisibleInstances(const glm::vec3& cameraPos,
                                const Frustum& frustum) {
        visibleInstanceMats.clear();
        float maxDistSq = maxRenderDistance * maxRenderDistance;

        for (size_t i = 0; i < InstanceMats.size(); i++) {
            const glm::vec3& pos = instancePositions[i];

            // Distance culling (use bounding radius for more accurate check)
            if (enableDistanceCulling) {
                float distSq = glm::dot(pos - cameraPos, pos - cameraPos);
                // Add bounding radius to effective max distance
                float effectiveMaxDist =
                    maxRenderDistance + cullingBoundingRadius;
                if (distSq > effectiveMaxDist * effectiveMaxDist) {
                    continue;
                }
            }

            // Frustum culling using bounding sphere
            if (enableFrustumCulling &&
                !frustum.isSphereInside(pos, cullingBoundingRadius)) {
                continue;
            }

            visibleInstanceMats.push_back(InstanceMats[i]);
        }
    }

    void deserialize(const nlohmann::json& data) override;
};

}  // namespace our