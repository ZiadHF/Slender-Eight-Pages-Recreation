#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"
#include "mtl-material-registry.hpp"
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
            

        }
    }

    // This function reads the lit material data from a json object
    // Priority: 1. MTL file values (if found), 2. JSON values, 3. Class defaults
    void LitMaterial::deserialize(const nlohmann::json &data)
    {
        std::cout << "Deserializing LitMaterial: " << materialName << std::endl;
        TexturedMaterial::deserialize(data);
        if (!data.is_object())
            return;

        // Try to get values from MTL registry using materialName (set by asset loader from JSON key)
        std::optional<MTLMaterialProperties> mtlProps;
        if (!materialName.empty())
        {
            std::cout << "Looking up MTL Material: " << materialName << std::endl;
            mtlProps = MTLMaterialRegistry::getInstance().getMaterial(materialName);
            if (mtlProps.has_value())
            {
                std::cout << "MTL Material found: " << materialName << std::endl;
            }
            else
            {
                std::cout << "MTL Material not found: " << materialName << std::endl;
            }
        }

        // Apply MTL values first (highest priority if they exist)
        if (mtlProps.has_value())
        {
            std::cout << "Applying MTL Material properties for: " << materialName << std::endl;
            std::cout << "MTL NAME IS: " << mtlProps->name << std::endl;
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
            std::cout << "Applied texture scale: (" << diffuseTextureScale.x << ", " << diffuseTextureScale.y << ", " << diffuseTextureScale.z << ")" << std::endl;
            

        }

        // JSON values only override if MTL wasn't found (fallback/defaults)
        if (!mtlProps.has_value())
        {
            std::cout << "Applying JSON Material properties for: " << materialName << std::endl;
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
    }

}