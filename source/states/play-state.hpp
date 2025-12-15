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
#include "../common/debug-utils.hpp"


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

    bool paused = false;

    void onInitialize() override {
        // Reset pause state
        paused = false;
        
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
        footstepSystem.initialize(&world, &physicsSystem);
        ambientTensionSystem.initialize(&world);
        staticSoundSystem.initialize(&world);
        glm::vec2 centerPos = glm::vec2(size.x / 2.0f - 75, size.y / 2.0f);
        textRenderer->startTimedText("Collect " + std::to_string(pageSystem.totalPages) + " Pages", 15.0f, centerPos, 0.5f,
                                     glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void onDraw(double deltaTime) override {
        // Get keyboard reference at the start
        auto& keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            // Lock mouse after
            getApp()->getMouse().lockMouse(getApp()->getWindow());
            // Manually sync mouse position to where lockMouse centered it
            int width, height;
            glfwGetWindowSize(getApp()->getWindow(), &width, &height);
            double centerX = width / 2.0;
            double centerY = height / 2.0;

            // Update the mouse's internal position tracking
            getApp()->getMouse().CursorMoveEvent(centerX, centerY);
            getApp()->getMouse().update();
            paused = !paused;
        }

        if (paused) {
            // Remove navigation logic - just check for ENTER to end game
            if (keyboard.justPressed(GLFW_KEY_ENTER)) {
                paused = false;
                getApp()->changeState("menu"); // End Game
            }

            // Unlock mouse
            getApp()->getMouse().unlockMouse(getApp()->getWindow());

            // Render game state frozen
            renderer.render(&world, 0.0f);

            // Render pause menu
            auto size = getApp()->getFrameBufferSize();
            glm::mat4 projection = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
            
            // Title
            std::string pauseTitle = "PAUSED";
            glm::vec2 titleSize = textRenderer->measureText(pauseTitle, 1.0f);
            glm::vec2 titlePos = glm::vec2(size.x / 2.0f - titleSize.x / 2.0f, size.y / 2.0f - 100);
            textRenderer->renderText(pauseTitle, titlePos, 1.0f, glm::vec4(1.0f), projection);

            // End Game option
            std::string endText = "Press ENTER to End Game";
            glm::vec2 endSize = textRenderer->measureText(endText, 0.7f);
            glm::vec2 endPos = glm::vec2(size.x / 2.0f - endSize.x / 2.0f, size.y / 2.0f);
            textRenderer->renderText(endText, endPos, 0.7f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), projection);

            // ESC hint
            std::string escText = "Press ESC to unpause";
            glm::vec2 escSize = textRenderer->measureText(escText, 0.5f);
            glm::vec2 escPos = glm::vec2(size.x / 2.0f - escSize.x / 2.0f, size.y / 2.0f + 70);
            textRenderer->renderText(escText, escPos, 0.5f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), projection);

            return; // Skip game updates
        }

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

        // DEBUG: Print position
        if (our::g_debugMode && keyboard.justPressed(GLFW_KEY_P)) {
            std::cout << "Player Position: (" << cameraPos.x << ", " << cameraPos.y
                      << ", " << cameraPos.z << ")\n";
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
        renderer.render(&world, (float)deltaTime);
        auto size = getApp()->getFrameBufferSize();
        glm::mat4 projection =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
        textRenderer->renderTimedTexts(projection);

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