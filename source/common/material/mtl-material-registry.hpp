#pragma once

#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>
#include <optional>

namespace our
{

    // Structure to hold material properties loaded from MTL files
    struct MTLMaterialProperties
    {
        std::string name;
        glm::vec3 ambient = glm::vec3(0.1f);  // Ka
        glm::vec3 diffuse = glm::vec3(0.8f);  // Kd
        glm::vec3 specular = glm::vec3(0.5f); // Ks
        float shininess = 32.0f;              // Ns (converted from MTL's 0-1000 range)
        float dissolve = 1.0f;                // d (opacity)
        int illuminationModel = 2;            // illum
        std::string diffuseTexture;           // map_Kd
        std::string specularTexture;          // map_Ks
        std::string normalTexture;            // map_Bump or bump
        
        // Texture scaling from MTL -s option (default 1,1,1 = no scaling)
        glm::vec3 diffuseTextureScale = glm::vec3(1.0f);   // map_Kd -s
        glm::vec3 specularTextureScale = glm::vec3(1.0f);  // map_Ks -s
        glm::vec3 normalTextureScale = glm::vec3(1.0f);    // map_Bump -s
        float bumpMultiplier = 1.0f;                        // map_Bump -bm
    };

    // Singleton registry to store MTL material properties globally
    // This allows materials to look up their properties when deserializing
    class MTLMaterialRegistry
    {
    public:
        static MTLMaterialRegistry &getInstance()
        {
            static MTLMaterialRegistry instance;
            return instance;
        }

        // Register material properties from an MTL file
        void registerMaterial(const std::string &name, const MTLMaterialProperties &props)
        {
            materials[name] = props;
        }

        // Get material properties by name (returns nullopt if not found)
        std::optional<MTLMaterialProperties> getMaterial(const std::string &name) const
        {
            auto it = materials.find(name);
            if (it != materials.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        // Check if a material exists in the registry
        bool hasMaterial(const std::string &name) const
        {
            return materials.find(name) != materials.end();
        }

        // Clear all registered materials
        void clear()
        {
            materials.clear();
        }

    private:
        MTLMaterialRegistry() = default;
        ~MTLMaterialRegistry() = default;
        MTLMaterialRegistry(const MTLMaterialRegistry &) = delete;
        MTLMaterialRegistry &operator=(const MTLMaterialRegistry &) = delete;

        std::unordered_map<std::string, MTLMaterialProperties> materials;
    };

}
