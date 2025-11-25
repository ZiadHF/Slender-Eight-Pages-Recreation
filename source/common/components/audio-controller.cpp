#define MINIAUDIO_IMPLEMENTATION
#include "audio-controller.hpp"

#include <iostream>

namespace our {

AudioController::AudioController() {
    ma_result result = ma_engine_init(NULL, &audioEngine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine." << std::endl;
    }
    isInitialized = (result == MA_SUCCESS);
}

AudioController::~AudioController() {
    if (musicStarted) {
        ma_sound_uninit(&audioSound);
    }
    if (isInitialized) {
        ma_engine_uninit(&audioEngine);
    }
}

bool AudioController::initializeMusic(const char* filename, bool loop) {
    if (!isInitialized) {
        return false;
    }

    ma_result result = ma_sound_init_from_file(
        &audioEngine, filename, MA_SOUND_FLAG_STREAM, NULL, NULL, &audioSound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load background music: " << filename
                  << std::endl;
        return false;
    }

    isLooping = loop;
    ma_sound_set_looping(&audioSound, loop ? MA_TRUE : MA_FALSE);
    return true;
}

void AudioController::setVolume(float vol) {
    if (!isInitialized) {
        return;
    }

    volume = vol;
    ma_sound_set_volume(&audioSound, volume);
}

void AudioController::playMusic() {
    if (!isInitialized) {
        return;
    }

    setVolume(volume);
    ma_sound_start(&audioSound);
    musicStarted = true;
}

void AudioController::stopMusic() {
    if (!isInitialized || !musicStarted) {
        return;
    }

    ma_sound_stop(&audioSound);
    musicStarted = false;
}

void AudioController::deserialize(const nlohmann::json& data) {
    if (!data.is_object()) return;

    if (data.contains("musicFile") && data["musicFile"].is_string()) {
        std::string filename = data["musicFile"];
        bool loop = data.value("loop", false);
        if (initializeMusic(filename.c_str(), loop)) {
            setVolume(data.value("volume", 1.0f));
            playMusic();
        }
    }
}

}  // namespace our