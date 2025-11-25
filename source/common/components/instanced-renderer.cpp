#include "instanced-renderer.hpp"
#include "../asset-loader.hpp"
#include "../texture/tree-utils.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
namespace our
{
    // Receives the mesh & material from the AssetLoader by the names given in the json object
    void InstancedRendererComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        
        std::cout << "Deserializing InstancedRenderer component..." << std::endl;
        
        mesh = AssetLoader<Mesh>::get(data["mesh"].get<std::string>());
        material = AssetLoader<Material>::get(data["material"].get<std::string>());
        
        std::cout << "Mesh: " << (mesh ? "loaded" : "NULL") << std::endl;
        std::cout << "Material: " << (material ? "loaded" : "NULL") << std::endl;
        
        if (data.contains("Map"))
        {
            std::string filename = data["Map"];
            float density = data.value("Density", 0.02f);
            glm::vec2 worldSize = glm::vec2(data.value("worldWidth", 400.0f), data.value("worldHeight", 400.0f));
            
            std::cout << "Generating trees from map..." << std::endl;
            auto instances = generateFromMap(filename, worldSize, density);
            
            std::cout << "Converting " << instances.size() << " instances to matrices..." << std::endl;
            for (const auto &inst : instances)
            {
                glm::mat4 transform = glm::translate(glm::mat4(1.0), inst.pos);
                transform = glm::rotate(transform, inst.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
                // Apply non-uniform scale: widthScale on X and Z, regular scale on all axes
                transform = glm::scale(transform, glm::vec3(inst.scale * inst.widthScale, inst.scale, inst.scale * inst.widthScale));
                InstanceMats.push_back(transform);
            }
            std::cout << "InstanceMats size: " << InstanceMats.size() << std::endl;
        }
    }

}