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

class AudioController : public Component {
   private:
    ma_engine audioEngine;
    ma_sound audioSound;
    bool isInitialized = false;
    bool musicStarted = false;

   public:
    bool isLooping = false;
    float volume = 1.0f;

    static std::string getID() { return "Audio Controller"; }

    AudioController();
    ~AudioController();
    bool initializeMusic(const char* filename, bool loop);
    void playMusic();
    void setVolume(float vol);
    void stopMusic();
    void deserialize(const nlohmann::json& data) override;
};

}  // namespace our