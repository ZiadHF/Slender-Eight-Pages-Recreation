#pragma once

#include <application.hpp>
#include <asset-loader.hpp>
#include <components/camera.hpp>
#include <ecs/world.hpp>
#include <systems/ambient-tension-system.hpp>
#include <systems/footstep-system.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/page-system.hpp>
#include <systems/physics-system.hpp>
#include <systems/slenderman-ai.hpp>
#include <systems/static-effect.hpp>
#include <systems/static-sound-system.hpp>

#include "../common/systems/text-renderer.hpp"
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
    our::FootstepSystem footstepSystem;
    our::AmbientTensionSystem ambientTensionSystem;
    our::StaticSoundSystem staticSoundSystem;
    our::TextRenderer* textRenderer;

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
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        textRenderer = new our::TextRenderer();
        // Initialize physics system
        physicsSystem.initialize(&world);
        cameraController.enter(getApp(), &physicsSystem);
        // Initialize Slenderman AI
        slendermanAISystem.initialize(&world);
        staticEffectSystem.initialize(&world);
        // Initialize page system with physics
        pageSystem.initialize(&world, &physicsSystem, textRenderer,
                              glm::vec2(size.x, size.y));
        // Initialize footstep system
        footstepSystem.initialize(&world);
        ambientTensionSystem.initialize(&world);
        staticSoundSystem.initialize(&world);
        glm::vec2 centerPos = glm::vec2(size.x / 2.0f - 75, size.y / 2.0f);
        textRenderer->startTimedText("Collect " + std::to_string(pageSystem.totalPages) + " Pages", 15.0f, centerPos, 0.5f,
                                     glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        slendermanAISystem.update(&world, (float)deltaTime, &renderer,
                                  &physicsSystem);
        staticEffectSystem.update(&world, &renderer);
        footstepSystem.update(&world, (float)deltaTime);
        ambientTensionSystem.update(&world, (float)deltaTime);
        staticSoundSystem.update(&world, (float)deltaTime);

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

        // Check for interact key
        bool interactPressed = false;
        // Check if interact key is a mouse button
        if (cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_LEFT ||
            cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_RIGHT ||
            cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_MIDDLE) {
            interactPressed = getApp()->getMouse().justPressed(
                cameraController.getInteractKey());
        } else {
            interactPressed = getApp()->getKeyboard().justPressed(
                cameraController.getInteractKey());
        }
        pageSystem.update(&world, (float)deltaTime, cameraPos, cameraForward,
                          interactPressed);

        textRenderer->updateTimedTexts((float)deltaTime);

        // And finally we use the renderer system to draw the scene
        renderer.render(&world);
        auto size = getApp()->getFrameBufferSize();
        glm::mat4 projection =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
        textRenderer->renderTimedTexts(projection);
        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }

        // Check if player has collected all pages
        if (pageSystem.allPagesCollected()) {
            getApp()->changeState("win");
        }
        else if (slendermanAISystem.playerIsDead()) {
            getApp()->changeState("death");
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
        // Destroy page system
        pageSystem.destroy();
        if (textRenderer) {
            textRenderer->clearTimedTexts();
            delete textRenderer;
            textRenderer = nullptr;
        }
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the
        // VRAM
        our::clearAllAssets();
    }
};