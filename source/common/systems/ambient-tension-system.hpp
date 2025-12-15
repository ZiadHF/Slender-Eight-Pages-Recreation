#pragma once
#include "../components/audio-controller.hpp"
#include "../components/slenderman.hpp"
#include "../ecs/world.hpp"

namespace our {

class AmbientTensionSystem {
   public:
    Entity* slenderman = nullptr;
    AudioController* ambienceAudio = nullptr;
    std::string currentAudioFile = "";
    bool firstPlay = true;
    float crossfadeDuration = 2.0f;  // 2 second crossfade for ambient

    void initialize(World* world) {
        for (auto entity : world->getEntities()) {
            if (entity->getComponent<SlendermanComponent>()) {
                slenderman = entity;

                // Get all audio components and find the ambience one
                auto audioComponents = entity->getComponents<AudioController>();
                for (auto* audio : audioComponents) {
                    if (audio->getAudioType() == AudioType::AMBIENCE) {
                        ambienceAudio = audio;
                        ambienceAudio->volume = 0.15f;
                        if (our::g_debugMode) {
                            std::cout
                                << "Ambient tension audio controller found."
                                << std::endl;
                        }
                        break;
                    }
                }
            }
        }
    }

    void setAudio(const std::string& audioFile) {
        if (ambienceAudio == nullptr) return;

        if (currentAudioFile != audioFile) {
            currentAudioFile = audioFile;

            if (firstPlay) {
                // First time, just play directly
                if (ambienceAudio->initializeMusic(audioFile.c_str(), true)) {
                    ambienceAudio->setVolume(0.15f);
                    ambienceAudio->playMusic();
                }
                firstPlay = false;
            } else {
                // Crossfade to new track
                ambienceAudio->crossfadeTo(audioFile.c_str(), true,
                                           crossfadeDuration);
            }
        }
    }

    void update(World* world, float deltaTime) {
        if (slenderman == nullptr || ambienceAudio == nullptr) return;

        // Update crossfade
        ambienceAudio->updateCrossfade(deltaTime);

        auto* slenderComp = slenderman->getComponent<SlendermanComponent>();
        if (slenderComp == nullptr) return;

        int currentAIValue = slenderComp->currentAIValue;

        // Depending on the current AI value, play different tension sounds
        if (currentAIValue >= slenderComp->maxAIValue * 0.75f) {
            setAudio("assets/sounds/tension_4.wav");
        } else if (currentAIValue >= slenderComp->maxAIValue * 0.5f) {
            setAudio("assets/sounds/tension_3.wav");
        } else if (currentAIValue >= slenderComp->maxAIValue * 0.25f) {
            setAudio("assets/sounds/tension_2.wav");
        } else if (currentAIValue > slenderComp->minAIValue + 1) {
            setAudio("assets/sounds/tension_1.wav");
        } else {
            setAudio("assets/sounds/wind.wav");
        }
    }
};

}  // namespace our