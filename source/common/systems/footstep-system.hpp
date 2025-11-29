#pragma once
#include "../components/audio-controller.hpp"
#include "../components/player.hpp"
#include "../ecs/world.hpp"

namespace our {
    class FootstepSystem {
       public:
        // Reference to player entity and its audio controller
        Entity* player = nullptr;
        AudioController* walkingAudio = nullptr;
        
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

        void initialize(World* world) {
            for (auto entity : world->getEntities()) {
                if (entity->getComponent<PlayerComponent>()) {
                    player = entity;
                    
                    // Get all audio components and find the walking one
                    auto audioComponents = entity->getComponents<AudioController>();
                    for (auto* audio : audioComponents) {
                        if (audio->getAudioType() == AudioType::WALKING) {
                            walkingAudio = audio;
                            std::cout << "Footstep audio controller found." << std::endl;
                            break;
                        }
                    }
                }
            }
        }

        void update(World* world, float deltaTime) {
            if (player == nullptr || walkingAudio == nullptr) return;
            auto* playerComp = player->getComponent<PlayerComponent>();
            if (playerComp == nullptr) return;

            // Update timer
            if (playerComp->isMoving) {
                footstepTimer += deltaTime;
                float interval = playerComp->isSprinting ? runStepInterval : walkStepInterval;

                if (footstepTimer >= interval) {
                    // Time to play footstep sound
                    footstepTimer = 0.0f;

                    // Determine surface type (currently always grass for simplicity)
                    bool onGrass = true; // Placeholder for actual surface detection
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