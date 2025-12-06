#pragma once

#include "../ecs/world.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/camera.hpp"
#include "../application.hpp"
#include "physics-system.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked
        PhysicsSystem* physics = nullptr; // Pointer to the physics system
        float playerEyeHeight = 1.6f; // Height from capsule bottom to camera

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app, PhysicsSystem* physicsSystem = nullptr) {
            this->app = app;
            this->physics = physicsSystem; 
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        void update(World* world, float deltaTime) {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent* controller = nullptr;
            Entity* entity = nullptr;
            
            for (auto e : world->getEntities()) {
                camera = e->getComponent<CameraComponent>();
                controller = e->getComponent<FreeCameraControllerComponent>();
                if (camera && controller) {
                    entity = e;
                    break;
                }
            }
            
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if (!(camera && controller && entity)) return;
            
            // Initialize player collider on first update - spawn higher to avoid clipping
            if (physics && !physics->isPlayerInitialized()) {
                glm::vec3 pos = glm::vec3(entity->getLocalToWorldMatrix()[3]);
                pos.y += 2.0f; // Spawn above ground to let gravity pull down
                physics->initializePlayerCollider(pos, 0.4f, 1.8f);
                std::cout << "Player collider initialized at: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
            }
            
            glm::vec3& position = entity->localTransform.position;
            glm::vec3& rotation = entity->localTransform.rotation;

            // Mouse look
            if (app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)) 
                app->getMouse().lockMouse(app->getWindow());
            else 
                app->getMouse().unlockMouse(app->getWindow());
            
            if (app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)) {
                glm::vec2 delta = app->getMouse().getMouseDelta();
                rotation.x -= delta.y * controller->rotationSensitivity;
                rotation.y -= delta.x * controller->rotationSensitivity;
            }

            rotation.x = glm::clamp(rotation.x, -glm::half_pi<float>() * 0.99f, 
                                                glm::half_pi<float>() * 0.99f);
            
            // Movement vectors (horizontal only)
            // Forward is along -Z when rotation.y = 0
            glm::vec3 front = glm::normalize(glm::vec3(-glm::sin(rotation.y), 0, -glm::cos(rotation.y)));
            glm::vec3 right = glm::normalize(glm::vec3(glm::cos(rotation.y), 0, -glm::sin(rotation.y)));
            
            glm::vec3 current_sensitivity = controller->positionSensitivity;
            if (app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT))
                current_sensitivity *= controller->speedupFactor;
            
            glm::vec3 moveDir(0.0f);
            if (app->getKeyboard().isPressed(GLFW_KEY_W)) moveDir += front;
            if (app->getKeyboard().isPressed(GLFW_KEY_S)) moveDir -= front;
            if (app->getKeyboard().isPressed(GLFW_KEY_D)) moveDir += right;
            if (app->getKeyboard().isPressed(GLFW_KEY_A)) moveDir -= right;
            
            if (glm::length(moveDir) > 0.001f)
                moveDir = glm::normalize(moveDir);
            
            if (physics && physics->isPlayerInitialized()) {
                // Physics-based movement
                physics->movePlayer(moveDir * current_sensitivity.x * deltaTime);
                
                // Get physics position and offset for eye height
                glm::vec3 physPos = physics->getPlayerPosition();
                position = physPos + glm::vec3(0, 0.5f, 0);
                
            } else {
                // Direct movement fallback
                position += moveDir * current_sensitivity * deltaTime;
            }
            
            // FOV control
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            camera->fovY = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f);
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit() {
            if (app) app->getMouse().unlockMouse(app->getWindow());
        }

    };

}
