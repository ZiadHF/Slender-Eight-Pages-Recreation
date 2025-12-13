#pragma once
#include "../components/audio-controller.hpp"
#include "../components/player.hpp"
#include "../ecs/world.hpp"
#include "../components/mesh-renderer.hpp"
#include "./physics-system.hpp"

namespace our {
    class FootstepSystem {
       public:
        // Reference to player entity and its audio controller
        Entity* player = nullptr;
        AudioController* walkingAudio = nullptr;
        AudioController* staminaAudio = nullptr;
        PhysicsSystem* physicsSystem = nullptr;

        // Boolean to track if stamina is depleted
        bool isStaminaDepleted = false;
        
        // Sound files for walking
        std::vector<std::string> grassSounds = {
            "assets/sounds/step1.wav",
            "assets/sounds/step2.wav",
        };
        std::vector<std::string> tileSounds = {
            "assets/sounds/tilestep1.wav",
            "assets/sounds/tilestep2.wav",
        };

        // Alternating indices
        int grassIndex = 0;
        int tileIndex = 0;

        // Timer to control footstep sound frequency
        float footstepTimer = 0.0f;
        float walkStepInterval = 0.7f; // seconds between footsteps
        float runStepInterval = 0.4f;  // seconds between footsteps when running

        void initialize(World* world, PhysicsSystem* physicsSys) {
            for (auto entity : world->getEntities()) {
                if (entity->getComponent<PlayerComponent>()) {
                    player = entity;
                    
                    // Get all audio components and find the walking and stamina ones
                    auto audioComponents = entity->getComponents<AudioController>();
                    for (auto* audio : audioComponents) {
                        if (audio->getAudioType() == AudioType::WALKING) {
                            walkingAudio = audio;
                            std::cout << "Footstep audio controller found." << std::endl;
                        } else if (audio->getAudioType() == AudioType::STAMINA) {
                            staminaAudio = audio;
                            std::cout << "Stamina audio controller found." << std::endl;
                        }
                    }
                }
            }
            physicsSystem = physicsSys;
        }
        
        void update(World* world, float deltaTime) {
            if (player == nullptr || walkingAudio == nullptr || staminaAudio == nullptr || physicsSystem == nullptr ) return;
            auto* playerComp = player->getComponent<PlayerComponent>();
            if (playerComp == nullptr) return;

            // Check for stamina depletion
            if (playerComp->stamina <= 0.0f){
                isStaminaDepleted = true;
            }

            if (isStaminaDepleted) {
                // Play stamina warning sound if not already playing
                if (!staminaAudio->isMusicStarted()) {
                    staminaAudio->initializeMusic("assets/sounds/breath.wav", true);
                    staminaAudio->setVolume(0.2f);
                    staminaAudio->playMusic();
                }
                // If stamina has recovered, stop the warning sound
                if (playerComp->stamina >= playerComp->maxStamina * 0.5f) {
                    isStaminaDepleted = false;
                    staminaAudio->stopMusic();
                    staminaAudio->uninitializeMusic();
                }
            }

            // Update timer
            if (playerComp->isMoving) {
                footstepTimer += deltaTime;
                float interval = playerComp->isSprinting ? runStepInterval : walkStepInterval;

                if (footstepTimer >= interval) {
                    // Time to play footstep sound
                    footstepTimer = 0.0f;

                    bool onGrass = true;
                    // Determine surface type using raycasting
                    glm::vec3 playerPos = physicsSystem->getPlayerPosition();
                    glm::vec3 rayStart = playerPos;
                    glm::vec3 rayEnd = playerPos + glm::vec3(0.0f, 5.0f, 0.0f);

                    RaycastResult hit = physicsSystem->raycast(rayStart, rayEnd);
                    if (hit.hit) {
                        // Check if under roof -> play tile sounds
                        if (hit.submeshName.find("Roof") != std::string::npos) {
                            onGrass = false;
                        }
                    }
                    
                    std::string soundFile;
                    if (onGrass) {
                        soundFile = grassSounds[grassIndex];
                        grassIndex = 1 - grassIndex;
                    } else {
                        soundFile = tileSounds[tileIndex];
                        tileIndex = 1 - tileIndex;
                    }
                    // Stop any currently playing footstep sound
                    walkingAudio->uninitializeMusic();
                    // Set and play sound
                    walkingAudio->initializeMusic(soundFile.c_str(), false);
                    walkingAudio->setVolume(0.1f);
                    walkingAudio->playMusic();
                }
            } else {
                footstepTimer = 0.0f; // Reset timer if not moving
            }
        }
    };
}