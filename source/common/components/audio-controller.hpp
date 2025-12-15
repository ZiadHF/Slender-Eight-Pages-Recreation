#pragma once

#include <miniaudio.h>

#include "../ecs/component.hpp"

// I encountered build issues due to the 'near' and 'far' macros defined in
// Windows headers conflicting with variable names in this file. To resolve
// this, I undefine these macros.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

namespace our {

enum class AudioType {
    MUSIC,
    AMBIENCE,
    STAMINA,
    WALKING,
    FLASHLIGHT,
    PAGE_COLLECT,
    STATIC
};

class AudioController : public Component {
   private:
    ma_engine audioEngine;

    // Dual sounds for crossfading
    ma_sound soundA;
    ma_sound soundB;
    bool soundAActive = true;  // Which sound is currently primary
    bool soundAInitialized = false;
    bool soundBInitialized = false;

    std::string audioFile;
    bool isInitialized = false;
    bool musicStarted = false;
    AudioType audioType = AudioType::MUSIC;

    // Crossfade state
    bool isCrossfading = false;
    float fadeTimeRemaining = 0.0f;
    float fadeDuration = 1.0f;  // seconds

   public:
    bool isLooping = false;
    float volume = 1.0f;

    static std::string getID() { return "Audio Controller"; }

    AudioType getAudioType() const { return audioType; }
    bool isMusicStarted() const { return musicStarted; }

    AudioController();
    ~AudioController();
    bool initializeMusic(const char* filename, bool loop);
    void uninitializeMusic();
    std::string getAudioFile() const;
    void playMusic();
    void setVolume(float vol);
    void stopMusic();

    // Crossfade methods
    bool crossfadeTo(const char* filename, bool loop, float duration = 1.0f);
    void updateCrossfade(float deltaTime);

    void deserialize(const nlohmann::json& data) override;
};

}  // namespace our