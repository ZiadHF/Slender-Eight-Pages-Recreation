#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp> 
#include "../mesh/mesh.hpp"
#include "../material/material.hpp"
#include <unordered_map>

namespace our {

    class InstancedRendererComponent : public Component {
    public:
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        std::vector<glm::mat4> InstanceMats;
        std::unordered_map<std::string, Material*> submeshMaterials;
        
        // Configuration for instance transforms
        glm::vec3 scaleMultiplier = glm::vec3(1.0f);
        glm::vec2 scaleRandomRange = glm::vec2(0.0f);  // min, max offset from multiplier
        
        glm::vec3 rotationMultiplier = glm::vec3(0.0f);
        glm::vec3 rotationRandomRange = glm::vec3(0.0f);
        
        glm::vec3 positionOffset = glm::vec3(0.0f);
        glm::vec3 positionRandomRange = glm::vec3(0.0f);
      
        static std::string getID() { return "Instanced Renderer"; }

        Material* getMaterialForSubmesh(const std::string& submeshName) const {
            auto it = submeshMaterials.find(submeshName);
            return (it != submeshMaterials.end()) ? it->second : material;
        }
  
        void deserialize(const nlohmann::json& data) override;
    };

}