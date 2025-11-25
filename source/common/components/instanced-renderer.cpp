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

        mesh = AssetLoader<Mesh>::get(data["mesh"].get<std::string>());
        material = AssetLoader<Material>::get(data["material"].get<std::string>());

        // Load submesh materials if specified
        if (data.contains("submeshMaterials") && data["submeshMaterials"].is_object())
        {

            for (auto &[submeshName, materialName] : data["submeshMaterials"].items())
            {

                Material *mat = AssetLoader<Material>::get(materialName.get<std::string>());
                if (mat)
                {
                    submeshMaterials[submeshName] = mat;
                }
            }
        }

        if (data.contains("Map"))
        {
            std::string filename = data["Map"];
            float density = data.value("Density", 0.02f);
            glm::vec2 worldSize = glm::vec2(data.value("worldWidth", 400.0f), data.value("worldHeight", 400.0f));

            // Load scale configuration (optional)
            bool hasScaleMultiplier = false;
            if (data.contains("scaleMultiplier")) {
                auto s = data["scaleMultiplier"];
                scaleMultiplier = glm::vec3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
                hasScaleMultiplier = true;
            }
            if (data.contains("scaleRandomRange")) {
                auto s = data["scaleRandomRange"];
                scaleRandomRange = glm::vec2(s[0].get<float>(), s[1].get<float>());
            }
            
            // Load rotation configuration (optional)
            bool hasRotationMultiplier = false;
            if (data.contains("rotationMultiplier")) {
                auto r = data["rotationMultiplier"];
                rotationMultiplier = glm::vec3(r[0].get<float>(), r[1].get<float>(), r[2].get<float>());
                hasRotationMultiplier = true;
            }
            if (data.contains("rotationRandomRange")) {
                auto r = data["rotationRandomRange"];
                rotationRandomRange = glm::vec3(r[0].get<float>(), r[1].get<float>(), r[2].get<float>());
            }
            
            // Load position configuration (optional)
            bool hasPositionOffset = false;
            if (data.contains("positionOffset")) {
                auto p = data["positionOffset"];
                positionOffset = glm::vec3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                hasPositionOffset = true;
            }
            if (data.contains("positionRandomRange")) {
                auto p = data["positionRandomRange"];
                positionRandomRange = glm::vec3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
            }

            auto instances = generateFromMap(filename, worldSize, density, positionRandomRange, rotationRandomRange, scaleRandomRange);

            for (const auto &inst : instances)
            {
                // Apply position
                glm::vec3 finalPos = inst.pos + inst.posRandom;
                if (hasPositionOffset) {
                    finalPos += positionOffset;
                }
                
                // Apply rotation
                glm::vec3 finalRotation = glm::vec3(0.0f, inst.rotation, 0.0f) + inst.rotationRandom;
                if (hasRotationMultiplier) {
                    finalRotation += rotationMultiplier;
                }
                
                // Apply scale
                glm::vec3 baseScale = glm::vec3(inst.scale * inst.widthScale, inst.scale, inst.scale * inst.widthScale);
                glm::vec3 finalScale = baseScale * (1.0f + inst.scaleRandom);
                if (hasScaleMultiplier) {
                    finalScale *= scaleMultiplier;
                }
                
                glm::mat4 transform = glm::translate(glm::mat4(1.0), finalPos);
                transform = glm::rotate(transform, finalRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                transform = glm::rotate(transform, finalRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                transform = glm::rotate(transform, finalRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
                transform = glm::scale(transform, finalScale);
                
                InstanceMats.push_back(transform);
            }
        }
    }

}