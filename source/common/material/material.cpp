#include "material.hpp"

#include "../asset-loader.hpp"
#include "../texture/texture-utils.hpp"
#include "deserialize-utils.hpp"
#include "mtl-material-registry.hpp"
#include "../debug-utils.hpp"
#include <iostream>
namespace our
{

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const
    {
        pipelineState.setup();
        if (shader)
        {
            shader->use();
        }
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        if (data.contains("pipelineState"))
        {
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint
    void TintedMaterial::setup() const
    {
        Material::setup();
        if (shader)
        {
            shader->set("tint", tint);
        }
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex"
    void TexturedMaterial::setup() const
    {
        TintedMaterial::setup();
        if (shader)
        {
            shader->set("alphaThreshold", alphaThreshold);
            // Explicitly bind diffuse texture to unit 0
            glActiveTexture(GL_TEXTURE0);
            if (texture)
            {
                texture->bind();
            }
            if (sampler)
            {
                sampler->bind(0);
            }
            // Bind to texture unit 0
            shader->set("tex", 0);
        }
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json &data)
    {
        TintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    // This function should call the setup of its parent and
    // set the lighting-related uniforms for Blinn-Phong shading
    void LitMaterial::setup() const
    {
        TexturedMaterial::setup();
        if (shader)
        {
            shader->set("ambient_color", ambient);
            shader->set("diffuse_color", diffuse);
            shader->set("specular_color", specular);
            shader->set("shininess", shininess);
            shader->set("illuminationModel", illuminationModel);
            // Send texture scale to shader (use xy for 2D textures)
            shader->set("textureScale", glm::vec2(diffuseTextureScale.x, diffuseTextureScale.y));
            
            // Normal mapping (texture unit 1)
            shader->set("hasNormalMap", hasNormalMap);
            if (hasNormalMap && normalMap) {
                glActiveTexture(GL_TEXTURE1);
                normalMap->bind();
                shader->set("normalMap", 1);
                shader->set("normalTextureScale", glm::vec2(normalTextureScale.x, normalTextureScale.y));
                shader->set("bumpMultiplier", bumpMultiplier);
            }
            
            // Specular map (texture unit 2)
            shader->set("hasSpecularMap", hasSpecularMap);
            if (hasSpecularMap && specularMap) {
                glActiveTexture(GL_TEXTURE2);
                specularMap->bind();
                shader->set("specularMap", 2);
            }
            
            // Roughness map (texture unit 3)
            shader->set("hasRoughnessMap", hasRoughnessMap);
            if (hasRoughnessMap && roughnessMap) {
                glActiveTexture(GL_TEXTURE3);
                roughnessMap->bind();
                shader->set("roughnessMap", 3);
            }
            
            // Ambient occlusion map (texture unit 4)
            shader->set("hasAoMap", hasAoMap);
            if (hasAoMap && aoMap) {
                glActiveTexture(GL_TEXTURE4);
                aoMap->bind();
                shader->set("aoMap", 4);
            }
            
            // Emissive map (texture unit 5)
            shader->set("hasEmissiveMap", hasEmissiveMap);
            if (hasEmissiveMap && emissiveMap) {
                glActiveTexture(GL_TEXTURE5);
                emissiveMap->bind();
                shader->set("emissiveMap", 5);
            }
            
            // Restore active texture unit to 0 for other operations
            glActiveTexture(GL_TEXTURE0);
        }
    }

    // This function reads the lit material data from a json object
    // Priority for texture maps: 1. JSON values (highest), 2. MTL file values, 3. No texture (use uniforms)
    // Priority for color values: 1. MTL file values (highest), 2. JSON values, 3. Class defaults
    void LitMaterial::deserialize(const nlohmann::json &data)
    {
        if (our::g_debugMode) std::cout << "Deserializing LitMaterial: " << materialName << std::endl;
        TexturedMaterial::deserialize(data);
        if (!data.is_object())
            return;

        // Try to get values from MTL registry using materialName (set by asset loader from JSON key)
        std::optional<MTLMaterialProperties> mtlProps;
        if (!materialName.empty())
        {
            if (our::g_debugMode) std::cout << "Looking up MTL Material: " << materialName << std::endl;
            mtlProps = MTLMaterialRegistry::getInstance().getMaterial(materialName);
            if (mtlProps.has_value())
            {
                if (our::g_debugMode) std::cout << "MTL Material found: " << materialName << std::endl;
            }
            else
            {
                if (our::g_debugMode) std::cout << "MTL Material not found: " << materialName << std::endl;
            }
        }

        // Apply MTL values first for color properties (highest priority if they exist)
        if (mtlProps.has_value())
        {
            if (our::g_debugMode) std::cout << "Applying MTL Material properties for: " << materialName << std::endl;
            if (our::g_debugMode) std::cout << "MTL NAME IS: " << mtlProps->name << std::endl;
            ambient = mtlProps->ambient;
            diffuse = mtlProps->diffuse;
            specular = mtlProps->specular;
            shininess = mtlProps->shininess;
            illuminationModel = mtlProps->illuminationModel;
            // Texture scaling from MTL -s option
            diffuseTextureScale = mtlProps->diffuseTextureScale;
            specularTextureScale = mtlProps->specularTextureScale;
            normalTextureScale = mtlProps->normalTextureScale;
            bumpMultiplier = mtlProps->bumpMultiplier;
            if (our::g_debugMode) std::cout << "Applied texture scale: (" << diffuseTextureScale.x << ", " << diffuseTextureScale.y << ", " << diffuseTextureScale.z << ")" << std::endl;
        }

        // JSON values only override color properties if MTL wasn't found (fallback/defaults)
        if (!mtlProps.has_value())
        {
            if (our::g_debugMode) std::cout << "Applying JSON Material properties for: " << materialName << std::endl;
            // Parse ambient color from JSON
            if (data.contains("ambient"))
            {
                auto &a = data["ambient"];
                ambient = glm::vec3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
            }

            // Parse diffuse color from JSON
            if (data.contains("diffuse"))
            {
                auto &d = data["diffuse"];
                diffuse = glm::vec3(d[0].get<float>(), d[1].get<float>(), d[2].get<float>());
            }

            // Parse specular color from JSON
            if (data.contains("specular"))
            {
                auto &s = data["specular"];
                specular = glm::vec3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
            }

            // Parse shininess from JSON
            if (data.contains("shininess"))
            {
                shininess = data["shininess"].get<float>();
            }
        }
        
        // ========== TEXTURE MAPS: JSON > MTL priority ==========
        // Normal map: JSON takes priority over MTL
        std::string normalMapPath = data.value("normalMap", "");
        if (normalMapPath.empty() && mtlProps.has_value() && !mtlProps->normalTexture.empty()) {
            normalMapPath = mtlProps->normalTexture;
        }
        if (!normalMapPath.empty()) {
            if (our::g_debugMode) std::cout << "Loading normal map: " << normalMapPath << std::endl;
            normalMap = texture_utils::loadImage(normalMapPath, true);
            hasNormalMap = (normalMap != nullptr);
            if (our::g_debugMode) std::cout << "Normal map loaded: " << (hasNormalMap ? "yes" : "no") << std::endl;
        }
        
        // Specular map: JSON takes priority over MTL
        std::string specularMapPath = data.value("specularMap", "");
        if (specularMapPath.empty() && mtlProps.has_value() && !mtlProps->specularTexture.empty()) {
            specularMapPath = mtlProps->specularTexture;
        }
        if (!specularMapPath.empty()) {
            if (our::g_debugMode) std::cout << "Loading specular map: " << specularMapPath << std::endl;
            specularMap = texture_utils::loadImage(specularMapPath, true);
            hasSpecularMap = (specularMap != nullptr);
            if (our::g_debugMode) std::cout << "Specular map loaded: " << (hasSpecularMap ? "yes" : "no") << std::endl;
        }
        
        // Roughness map: JSON takes priority over MTL
        std::string roughnessMapPath = data.value("roughnessMap", "");
        if (roughnessMapPath.empty() && mtlProps.has_value() && !mtlProps->roughnessTexture.empty()) {
            roughnessMapPath = mtlProps->roughnessTexture;
        }
        if (!roughnessMapPath.empty()) {
            if (our::g_debugMode) std::cout << "Loading roughness map: " << roughnessMapPath << std::endl;
            roughnessMap = texture_utils::loadImage(roughnessMapPath, true);
            hasRoughnessMap = (roughnessMap != nullptr);
            if (our::g_debugMode) std::cout << "Roughness map loaded: " << (hasRoughnessMap ? "yes" : "no") << std::endl;
        }
        
        // Ambient occlusion map: JSON takes priority over MTL
        std::string aoMapPath = data.value("aoMap", "");
        if (aoMapPath.empty() && mtlProps.has_value() && !mtlProps->aoTexture.empty()) {
            aoMapPath = mtlProps->aoTexture;
        }
        if (!aoMapPath.empty()) {
            if (our::g_debugMode) std::cout << "Loading AO map: " << aoMapPath << std::endl;
            aoMap = texture_utils::loadImage(aoMapPath, true);
            hasAoMap = (aoMap != nullptr);
            if (our::g_debugMode) std::cout << "AO map loaded: " << (hasAoMap ? "yes" : "no") << std::endl;
        }
        
        // Emissive map: JSON takes priority over MTL
        std::string emissiveMapPath = data.value("emissiveMap", "");
        if (emissiveMapPath.empty() && mtlProps.has_value() && !mtlProps->emissiveTexture.empty()) {
            emissiveMapPath = mtlProps->emissiveTexture;
        }
        if (!emissiveMapPath.empty()) {
            if (our::g_debugMode) std::cout << "Loading emissive map: " << emissiveMapPath << std::endl;
            emissiveMap = texture_utils::loadImage(emissiveMapPath, true);
            hasEmissiveMap = (emissiveMap != nullptr);
            if (our::g_debugMode) std::cout << "Emissive map loaded: " << (hasEmissiveMap ? "yes" : "no") << std::endl;
        }
    }

}
