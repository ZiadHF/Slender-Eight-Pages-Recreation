#include "mesh-renderer.hpp"
#include "../asset-loader.hpp"

namespace our {
    // Receives the mesh & material from the AssetLoader by the names given in the json object
    void MeshRendererComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        mesh = AssetLoader<Mesh>::get(data["mesh"].get<std::string>());
        material = AssetLoader<Material>::get(data["material"].get<std::string>());
        
        // Auto-load submesh materials based on mesh submeshes
        if (mesh && mesh->getSubmeshCount() > 0) {
            for (const auto& submesh : mesh->getSubmeshes()) {
                Material* mat = AssetLoader<Material>::get(submesh.materialName);
                if (mat) {
                    submeshMaterials[submesh.materialName] = mat;
                }
            }
        }
        
        // Load submesh materials if specified manually in JSON
        if (data.contains("submeshMaterials") && data["submeshMaterials"].is_object()) {
            for (auto& [name, matName] : data["submeshMaterials"].items()) {
                Material* mat = AssetLoader<Material>::get(matName.get<std::string>());
                if (mat) {
                    submeshMaterials[name] = mat;
                }
            }
        }
    }
}