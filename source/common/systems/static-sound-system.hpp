#pragma once
#include "../components/audio-controller.hpp"
#include "../components/player.hpp"
#include "../ecs/world.hpp"

namespace our {
class StaticSoundSystem {
   public:
    Entity* player = nullptr;
    AudioController* staticSound = nullptr;
    std::string currentAudioFile = "";
    bool isPlaying = false;
    bool firstPlay = true;
    float crossfadeDuration = 1.0f;  // 1 second crossfade for static (faster)

    void initialize(World* world) {
        for (auto entity : world->getEntities()) {
            if (entity->getComponent<PlayerComponent>()) {
                player = entity;

                // Get all audio components and find the static sound one
                auto audioComponents = entity->getComponents<AudioController>();
                for (auto* audio : audioComponents) {
                    if (audio->getAudioType() == AudioType::STATIC) {
                        staticSound = audio;
                        staticSound->volume = 0.5f;
                        std::cout << "Static sound audio controller found."
                                  << std::endl;
                        break;
                    }
                }
            }
        }
    }

    void setAudio(const std::string& audioFile) {
        if (staticSound == nullptr) return;

        if (currentAudioFile != audioFile) {
            currentAudioFile = audioFile;

            if (firstPlay || !isPlaying) {
                // First time or coming from stopped state, just play directly
                if (staticSound->initializeMusic(audioFile.c_str(), true)) {
                    staticSound->setVolume(0.2f);
                    staticSound->playMusic();
                }
                firstPlay = false;
            } else {
                // Crossfade to new static level
                staticSound->crossfadeTo(audioFile.c_str(), true,
                                         crossfadeDuration);
            }
            isPlaying = true;
        }
    }

    void stopAudio() {
        if (isPlaying && staticSound != nullptr) {
            staticSound->stopMusic();
            staticSound->uninitializeMusic();
            currentAudioFile = "";
            isPlaying = false;
            firstPlay = true;  // Reset so next play starts fresh
        }
    }

    void update(World* world, float deltaTime) {
        if (staticSound == nullptr || player == nullptr) return;

        // Update crossfade
        staticSound->updateCrossfade(deltaTime);

        auto* playerComp = player->getComponent<PlayerComponent>();
        if (playerComp == nullptr) return;

        // Depending on the player's health, play different static sounds
        float healthPercent = playerComp->health / playerComp->maxHealth;
        if (healthPercent >= 1.0f) {
            // No static sound at full health
            stopAudio();
        } else if (healthPercent >= 0.75f) {
            setAudio("assets/sounds/static_light.wav");
        } else if (healthPercent >= 0.10f) {
            setAudio("assets/sounds/static_medium.wav");
        } else {
            setAudio("assets/sounds/static_heavy.wav");
        }
    }
};

}  // namespace our