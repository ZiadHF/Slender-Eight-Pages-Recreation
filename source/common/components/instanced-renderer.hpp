#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp> 
#include "../mesh/mesh.hpp"
#include "../material/material.hpp"

namespace our {

   
    class InstancedRendererComponent : public Component {
    public:
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        std::vector<glm::mat4> InstanceMats;
      
        static std::string getID() { return "Instanced Renderer"; }

  
        void deserialize(const nlohmann::json& data) override;
    };

}