#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_map>
#include <random>

#include "../common/components/page-spawner.hpp"
#include "../common/components/page.hpp"
#include "../common/components/player.hpp"
#include "../common/components/slenderman.hpp"
#include "../common/components/mesh-renderer.hpp"
#include "../common/ecs/entity.hpp"
#include "physics-system.hpp"

namespace our {

class PageSystem {
   public:
    Entity* player = nullptr;
    Entity* slenderman = nullptr;
    Entity* pageSpawner = nullptr;
    int totalPages = 0;
    std::vector<Entity*> spawnedPages;
    our::ShaderProgram* pageShader = nullptr;

    // Physics reference
    PhysicsSystem* physics = nullptr;

    // Map Entity to its collision body for cleanup
    std::unordered_map<Entity*, btRigidBody*> pageColliders;

    // Raycast parameters
    float interactionDistance = 3.0f;  // Max distance player can interact

    void initialize(World* world, PhysicsSystem* physicsSystem) {
        physics = physicsSystem;
        totalPages = 0;

        // Clear any existing data
        spawnedPages.clear();
        pageColliders.clear();
        if (pageShader) {
            delete pageShader;
            pageShader = nullptr;
        }

        for (auto entity : world->getEntities()) {
            if (entity->getComponent<PlayerComponent>()) {
                player = entity;
            }
            if (entity->getComponent<SlendermanComponent>()) {
                slenderman = entity;
            }
            if (entity->getComponent<PageSpawnerComponent>()) {
                pageSpawner = entity;
            }
        }

        if (!player || !slenderman || !pageSpawner) {
            std::cerr << "PageSystem: Player, Slenderman, or PageSpawner "
                         "entity not found!"
                      << std::endl;
        }

        // Get total pages and page textures from spawner component
        auto* spawnerComp = pageSpawner->getComponent<PageSpawnerComponent>();
        totalPages = spawnerComp->totalPages;
        std::vector<std::string> pageTextures = spawnerComp->pageTextures;

        // Choose random spawn locations from the spawner's list
        std::vector<std::pair<glm::vec3, glm::vec3>> spawnLocations = spawnerComp->spawnPoints;
        int size = spawnLocations.size();
        if (size < totalPages) {
            std::cerr
            << "PageSystem: Not enough spawn locations for total pages!"
            << std::endl;
            totalPages = size;
        }

        // Randomly shuffle and pick locations
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(spawnLocations.begin(), spawnLocations.end(), gen);


        std::cout << "Spawning " << totalPages << " pages." << std::endl;
        pageShader = new our::ShaderProgram();
        pageShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        pageShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        pageShader->link();

        for (int i = 0; i < totalPages; i++) {
            Entity* pageEntity = world->add();
            pageEntity->name = "Page_" + std::to_string(i);
            pageEntity->localTransform.position = spawnLocations[i].first;
            pageEntity->localTransform.rotation = spawnLocations[i].second;

            // Add MeshComponent
            auto* meshComp = pageEntity->addComponent<MeshRendererComponent>();
            meshComp->mesh = AssetLoader<Mesh>::get("page");
            // Create a material
            TexturedMaterial* pageMaterial = new TexturedMaterial();
            pageMaterial->shader = pageShader;
            pageMaterial->tint = glm::vec4(1.0f);
            pageMaterial->sampler = AssetLoader<Sampler>::get("default");
            pageMaterial->pipelineState.faceCulling.enabled = true;
            pageMaterial->pipelineState.depthTesting.enabled = true;
            pageMaterial->pipelineState.depthTesting.function = GL_LEQUAL;

            // Load a random texture for the page
            int rand = std::rand() % pageTextures.size();
            pageMaterial->texture = our::texture_utils::loadImage(pageTextures[rand]);
            pageTextures.erase(pageTextures.begin() + rand); // Ensure unique textures

            meshComp->material = pageMaterial;

            // Add PageComponent
            auto* pageComp = pageEntity->addComponent<PageComponent>();
            pageComp->isCollected = false;

            spawnedPages.push_back(pageEntity);

            // Register collider in physics system
            registerPageCollider(pageEntity);
        }
   

        std::cout << "PageSystem initialized with " << totalPages << " pages"
                  << std::endl;
    }

    void destroy() {
        // Destroy all uncollected pages
        for (Entity* page : spawnedPages) {
            auto* pageComp = page->getComponent<PageComponent>();
            if (pageComp && !pageComp->isCollected) {
                // Delete the material and texture
                auto* meshComp = page->getComponent<MeshRendererComponent>();
                if (meshComp) {
                    auto* material = dynamic_cast<TexturedMaterial*>(meshComp->material);
                    if (material) {
                        if (material->texture) {
                            delete material->texture;
                            material->texture = nullptr;
                        }
                        delete material;
                    }
                }

                // Remove mesh component
                page->deleteComponent<MeshRendererComponent>();
            }
        }
        spawnedPages.clear();
        
        // Cleanup page colliders
        for (auto& pair : pageColliders) {
            physics->removeBody(pair.second);
        }
        pageColliders.clear();

        // Cleanup shader
        if (pageShader) {
            delete pageShader;
            pageShader = nullptr;
        }
    }

    void registerPageCollider(Entity* entity) {
        if (!physics) return;

        glm::vec3 pagePos = glm::vec3(entity->getLocalToWorldMatrix()[3]);
        auto* pageComp = entity->getComponent<PageComponent>();

        // Add a sphere collider for the page
        btRigidBody* body = physics->addStaticSphere(
            pagePos, interactionDistance,
            entity  // Store entity pointer for identification
        );

        pageColliders[entity] = body;
    }

    // Call this when player looks at center of screen and clicks interact
    void update(World* world, float deltaTime, const glm::vec3& cameraPos,
                const glm::vec3& cameraForward, bool interactPressed) {
        if (!player || !physics) return;

        auto* playerComp = player->getComponent<PlayerComponent>();
        if (!playerComp) return;

        // Only check for interaction when player presses interact key
        if (!interactPressed) return;

        // Raycast from camera position in camera forward direction
        RaycastResult hit =
            physics->raycast(cameraPos, cameraForward, interactionDistance);

        if (hit.hit && hit.userData) {
            // Check if we hit a page entity
            Entity* hitEntity = static_cast<Entity*>(hit.userData);
            auto* pageComp = hitEntity->getComponent<PageComponent>();

            if (pageComp && !pageComp->isCollected) {
                collectPage(hitEntity, pageComp, playerComp);
            }
        }
    }

    void collectPage(Entity* entity, PageComponent* pageComp,
                     PlayerComponent* playerComp) {
        pageComp->isCollected = true;
        playerComp->collectedPages++;

        // Remove collider from physics world
        auto it = pageColliders.find(entity);
        if (it != pageColliders.end()) {
            physics->removeBody(it->second);
            pageColliders.erase(it);
        }
        
        // Delete the material and texture
        auto* meshComp = entity->getComponent<MeshRendererComponent>();
        if (meshComp) {
            auto* material = dynamic_cast<TexturedMaterial*>(meshComp->material);
            if (material) {
                if (material->texture) {
                    delete material->texture;
                    material->texture = nullptr;
                }
                delete material;
            }
        }

        // Remove mesh component
        entity->deleteComponent<MeshRendererComponent>();

        // Trigger effects
        onPageCollected();
    }

    bool allPagesCollected() const {
        return player->getComponent<PlayerComponent>()->collectedPages >=
               totalPages;
    }

   private:
    void onPageCollected() {
        // Increase Slenderman aggression
        if (slenderman) {
            auto* slenderComp = slenderman->getComponent<SlendermanComponent>();
            if (slenderComp) {
                slenderComp->teleportCooldown *= 0.9f;  // Teleports more often
            }
        }

        // Play collection sound
        auto* audioComp = pageSpawner->getComponent<AudioController>();
        if (audioComp) {
            audioComp->initializeMusic("assets/sounds/grab_page.wav", false);
            audioComp->setVolume(0.8f);
            audioComp->playMusic();
        }

        // TODO: Update UI
    }
};

}  // namespace our