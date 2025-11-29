#pragma once

#include <application.hpp>
#include <asset-loader.hpp>
#include <components/camera.hpp>
#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/page-system.hpp>
#include <systems/physics-system.hpp>
#include <systems/slenderman-ai.hpp>
#include <systems/static-effect.hpp>

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State {
    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::SlendermanAISystem slendermanAISystem;
    our::StaticEffectSystem staticEffectSystem;
    our::PhysicsSystem physicsSystem;
    our::PageSystem pageSystem;

    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets")) {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our
        // world
        if (config.contains("world")) {
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer
        // to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        // Initialize physics system
        physicsSystem.initialize();
        // Initialize Slenderman AI
        slendermanAISystem.initialize(&world);
        staticEffectSystem.initialize(&world);
        // Initialize page system with physics
        pageSystem.initialize(&world, &physicsSystem);
    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        slendermanAISystem.update(&world, (float)deltaTime, &renderer);
        staticEffectSystem.update(&world, &renderer);

        // Update physics
        physicsSystem.update((float)deltaTime);

        // Get camera position and forward for page interaction raycast
        glm::vec3 cameraPos(0), cameraForward(0, 0, -1);
        for (auto entity : world.getEntities()) {
            auto* camera = entity->getComponent<our::CameraComponent>();
            if (camera) {
                glm::mat4 matrix = entity->getLocalToWorldMatrix();
                cameraPos = glm::vec3(matrix[3]);
                cameraForward =
                    glm::normalize(glm::vec3(matrix * glm::vec4(0, 0, -1, 0)));
                break;
            }
        }

        // Check for interact key (E key)
        bool interactPressed = getApp()->getKeyboard().justPressed(GLFW_KEY_E);
        pageSystem.update(&world, (float)deltaTime, cameraPos, cameraForward,
                          interactPressed);

        if (getApp()->getKeyboard().isPressed(GLFW_KEY_P)) {
            glm::vec3 playerPos =
                glm::vec3(slendermanAISystem.player->getLocalToWorldMatrix()[3]);
            std::cout << "Player Position: (" << playerPos.x << ", "
                      << playerPos.y << ", " << playerPos.z << ")\n";
        }
        // And finally we use the renderer system to draw the scene
        renderer.render(&world, (float)deltaTime);

        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // Destroy physics system
        physicsSystem.destroy();
        // On exit, we call exit for the camera controller system to make sure
        // that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the
        // VRAM
        our::clearAllAssets();
    }
};