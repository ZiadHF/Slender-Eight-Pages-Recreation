#pragma once

#include <GLFW/glfw3.h>

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "../application.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/audio-controller.hpp"
#include "../components/player.hpp"
#include "../ecs/world.hpp"
#include "physics-system.hpp"
namespace our {

// The free camera controller system is responsible for moving every entity
// which contains a FreeCameraControllerComponent. This system is added as a
// slightly complex example for how use the ECS framework to implement logic.
// For more information, see "common/components/free-camera-controller.hpp"
class FreeCameraControllerSystem {
    Application* app;          // The application in which the state runs
    bool mouse_locked = true;  // Is the mouse locked
    PhysicsSystem* physics = nullptr;  // Pointer to the physics system
    float playerEyeHeight = 1.2f;      // Height from capsule bottom to camera
    PlayerComponent* playerComp = nullptr;
    float bobbingTime = 0.0f;        // Time accumulator for bobbing
    float bobbingFrequency = 10.0f;  // Cycles per second while walking
    float bobbingAmplitude = 0.14f;  // Vertical bob amount
    float bobbingSway = 0.09f;
    std::map<std::string, int> controlKeys;
    bool flashlightKeyWasPressed = false;  // For detecting key press edge

    void playFlashlightSound(AudioController* audio, const std::string& soundFile) {
        if (!audio) return;
        audio->uninitializeMusic();
        audio->initializeMusic(soundFile.c_str(), false);
        audio->setVolume(0.3f);
        audio->playMusic();
    }

    void loadControls() {
        std::ifstream file("config/player.json");
        if (file.is_open()) {
            nlohmann::json config;
            file >> config;
            file.close();

            if (config.contains("controls")) {
                auto controls = config["controls"];
                controlKeys["forward"] =
                    stringToGLFWKey(controls.value("forward", "W"));
                controlKeys["backward"] =
                    stringToGLFWKey(controls.value("backward", "S"));
                controlKeys["left"] =
                    stringToGLFWKey(controls.value("left", "A"));
                controlKeys["right"] =
                    stringToGLFWKey(controls.value("right", "D"));
                controlKeys["sprint"] =
                    stringToGLFWKey(controls.value("sprint", "LEFT_SHIFT"));
                controlKeys["interact"] =
                    stringToGLFWKey(controls.value("interact", "E"));
                controlKeys["toggle_flashlight"] =
                    stringToGLFWKey(controls.value("toggle_flashlight", "F"));
            }
        }
    }

    bool isKeyPressed(const std::string& action) {
        if (controlKeys.find(action) == controlKeys.end()) return false;
        int key = controlKeys[action];

        // Check if it's a mouse button
        if (key == GLFW_MOUSE_BUTTON_LEFT || key == GLFW_MOUSE_BUTTON_RIGHT ||
            key == GLFW_MOUSE_BUTTON_MIDDLE) {
            return app->getMouse().isPressed(key);
        }
        return app->getKeyboard().isPressed(key);
    }

   public:
    // When a state enters, it should call this function and give it the pointer
    // to the application
    void enter(Application* app, PhysicsSystem* physicsSystem = nullptr) {
        this->app = app;
        this->physics = physicsSystem;
        loadControls();
        mouse_locked = true;
        app->getMouse().lockMouse(app->getWindow());

        // Manually sync mouse position to where lockMouse centered it
        int width, height;
        glfwGetWindowSize(app->getWindow(), &width, &height);
        double centerX = width / 2.0;
        double centerY = height / 2.0;

        // Update the mouse's internal position tracking
        app->getMouse().CursorMoveEvent(centerX, centerY);
        app->getMouse().update();
        playerComp = nullptr;
    }

    // This should be called every frame to update all entities containing a
    // FreeCameraControllerComponent
    void update(World* world, float deltaTime) {
        // First of all, we search for an entity containing both a
        // CameraComponent and a FreeCameraControllerComponent As soon as we
        // find one, we break
        CameraComponent* camera = nullptr;
        FreeCameraControllerComponent* controller = nullptr;
        Entity* entity = nullptr;

        // Get player component for later use
        if (!playerComp) {
            for (auto entity : world->getEntities()) {
                auto pc = entity->getComponent<PlayerComponent>();
                if (pc) {
                    playerComp = pc;
                    break;
                }
            }
        }

        for (auto e : world->getEntities()) {
            camera = e->getComponent<CameraComponent>();
            controller = e->getComponent<FreeCameraControllerComponent>();
            if (camera && controller) {
                entity = e;
                break;
            }
        }

        // If there is no entity with both a CameraComponent and a
        // FreeCameraControllerComponent, we can do nothing so we return
        if (!(camera && controller && entity)) return;

        // Initialize player collider on first update - spawn higher to avoid
        // clipping
        if (physics && !physics->isPlayerInitialized()) {
            glm::vec3 pos = glm::vec3(entity->getLocalToWorldMatrix()[3]);
            // pos.y = glm::max(pos.y, 2.0f);
            pos.y += 2.0f;  // Spawn above ground to let gravity pull down
            physics->initializePlayerCollider(pos, 0.4f, playerEyeHeight);
            if (our::g_debugMode) {
            std::cout << "Player collider initialized at: " << pos.x << ", "
                      << pos.y << ", " << pos.z << std::endl;
            }
        }

        glm::vec3& position = entity->localTransform.position;
        glm::vec3& rotation = entity->localTransform.rotation;

        // Mouse look
        glm::vec2 delta = app->getMouse().getMouseDelta();
        rotation.x -= delta.y * controller->rotationSensitivity;
        rotation.y -= delta.x * controller->rotationSensitivity;

        rotation.x = glm::clamp(rotation.x, -glm::half_pi<float>() * 0.99f,
                                glm::half_pi<float>() * 0.99f);

        // Movement vectors (horizontal only)
        // Forward is along -Z when rotation.y = 0
        glm::vec3 front = glm::normalize(
            glm::vec3(-glm::sin(rotation.y), 0, -glm::cos(rotation.y)));
        glm::vec3 right = glm::normalize(
            glm::vec3(glm::cos(rotation.y), 0, -glm::sin(rotation.y)));

        // Set sprinting speed and state
        glm::vec3 current_sensitivity = playerComp->walkSpeed;
        if (isKeyPressed("sprint") && playerComp->isMoving) {
            if (playerComp->stamina > 0.0f) {
                current_sensitivity *= playerComp->sprintSpeedup;
                playerComp->isSprinting = true;
            }
            else {
                playerComp->isSprinting = false;
            }
            playerComp->stamina = std::max(0.0f, playerComp->stamina - playerComp->staminaDrainRate * deltaTime);
            playerComp->staminaRegenTimer = playerComp->staminaRegenDelay; // Reset regen delay
        } else {
            playerComp->isSprinting = false;
            // Only regenerate after delay has passed
            if (playerComp->staminaRegenTimer > 0.0f) {
                playerComp->staminaRegenTimer -= deltaTime;
            } else {
                playerComp->stamina = std::min(playerComp->maxStamina, playerComp->stamina + playerComp->staminaRegenRate * deltaTime);
            }
        }

        // Flashlight toggle
        bool flashlightKeyPressed = isKeyPressed("toggle_flashlight");
        if (flashlightKeyPressed && !flashlightKeyWasPressed) {
            playFlashlightSound(entity->getComponent<AudioController>(), "assets/sounds/flashlight_click.wav");
            playerComp->flashlightOn = !playerComp->flashlightOn;
        }
        flashlightKeyWasPressed = flashlightKeyPressed;

        glm::vec3 moveDir(0.0f);

        if (isKeyPressed("forward")) moveDir += front;
        if (isKeyPressed("backward")) moveDir -= front;
        if (isKeyPressed("right")) moveDir += right;
        if (isKeyPressed("left")) moveDir -= right;

        // Set moving state for footstep sounds
        if (glm::length(moveDir) > 0.001f) {
            moveDir = glm::normalize(moveDir);
            playerComp->isMoving = true;
        } else {
            playerComp->isMoving = false;
        }

        if (physics && physics->isPlayerInitialized()) {
            // Physics-based movement - Bullet handles timing internally via
            // stepSimulation
            physics->movePlayer(moveDir * current_sensitivity.x *
                                0.016f);  // Use fixed timestep (~60fps)

            // Get physics position and offset for eye height
            glm::vec3 physPos = physics->getPlayerPosition();
            glm::vec3 bobOffset(0.0f);
            if (playerComp->isMoving) {
                float speedMultiplier = playerComp->isSprinting ? 1.4f : 1.0f;
                bobbingTime += deltaTime * bobbingFrequency * speedMultiplier;

                // Vertical bobbing - complete cycle (up and down) per step
                bobOffset.y = glm::sin(bobbingTime) * bobbingAmplitude;

                // Horizontal sway - half the frequency for natural side-to-side
                bobOffset.x = glm::sin(bobbingTime * 0.5f) * bobbingSway;
            } else {
                // Smoothly decay bobbing when stopped (frame-rate independent)
                float decayRate = 10.0f;  // Higher = faster decay
                bobbingTime *= glm::exp(-decayRate * deltaTime);
            }

            position = physPos + glm::vec3(0, playerEyeHeight, 0) + bobOffset;
        } else {
            // Direct movement fallback
            position += moveDir * current_sensitivity * deltaTime;
        }
    }

    // When the state exits, it should call this function to ensure the mouse is
    // unlocked
    void exit() {
        if (app) app->getMouse().unlockMouse(app->getWindow());
    }

    int getInteractKey() const {
        auto it = controlKeys.find("interact");
        return (it != controlKeys.end()) ? it->second : GLFW_KEY_E;
    }
};

}  // namespace our
