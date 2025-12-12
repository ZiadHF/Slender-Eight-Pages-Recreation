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
    if (soundAInitialized) {
        ma_sound_uninit(&soundA);
    }
    if (soundBInitialized) {
        ma_sound_uninit(&soundB);
    }
    if (isInitialized) {
        ma_engine_uninit(&audioEngine);
    }
}

bool AudioController::initializeMusic(const char* filename, bool loop) {
    if (!isInitialized) {
        return false;
    }

    // Use the active sound slot
    ma_sound* activeSound = soundAActive ? &soundA : &soundB;
    bool* activeInitialized =
        soundAActive ? &soundAInitialized : &soundBInitialized;

    ma_result result = ma_sound_init_from_file(
        &audioEngine, filename, MA_SOUND_FLAG_STREAM, NULL, NULL, activeSound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load background music: " << filename
                  << std::endl;
        return false;
    }
    audioFile = filename;
    isLooping = loop;
    ma_sound_set_looping(activeSound, loop ? MA_TRUE : MA_FALSE);
    *activeInitialized = true;
    return true;
}

void AudioController::setVolume(float vol) {
    if (!isInitialized) {
        return;
    }

    volume = vol;
    ma_sound* activeSound = soundAActive ? &soundA : &soundB;
    bool activeInitialized =
        soundAActive ? soundAInitialized : soundBInitialized;
    if (activeInitialized) {
        ma_sound_set_volume(activeSound, volume);
    }
}

void AudioController::playMusic() {
    if (!isInitialized) {
        return;
    }

    ma_sound* activeSound = soundAActive ? &soundA : &soundB;
    bool activeInitialized =
        soundAActive ? soundAInitialized : soundBInitialized;
    if (activeInitialized) {
        setVolume(volume);
        ma_sound_start(activeSound);
        musicStarted = true;
    }
}

void AudioController::stopMusic() {
    if (!isInitialized || !musicStarted) {
        return;
    }

    if (soundAInitialized) {
        ma_sound_stop(&soundA);
    }
    if (soundBInitialized) {
        ma_sound_stop(&soundB);
    }
    musicStarted = false;
}

std::string AudioController::getAudioFile() const { return audioFile; }

void AudioController::uninitializeMusic() {
    if (soundAInitialized) {
        ma_sound_stop(&soundA);
        ma_sound_uninit(&soundA);
        soundAInitialized = false;
    }
    if (soundBInitialized) {
        ma_sound_stop(&soundB);
        ma_sound_uninit(&soundB);
        soundBInitialized = false;
    }
    audioFile = "";
    musicStarted = false;
    isCrossfading = false;
    soundAActive = true;
}

bool AudioController::crossfadeTo(const char* filename, bool loop,
                                  float duration) {
    if (!isInitialized) {
        return false;
    }

    // If same file, do nothing
    if (audioFile == filename) {
        return true;
    }

    // Determine which sound to fade in (the inactive one)
    ma_sound* fadeInSound = soundAActive ? &soundB : &soundA;
    bool* fadeInInitialized =
        soundAActive ? &soundBInitialized : &soundAInitialized;

    // Clean up the fade-in slot if it was previously used
    if (*fadeInInitialized) {
        ma_sound_stop(fadeInSound);
        ma_sound_uninit(fadeInSound);
        *fadeInInitialized = false;
    }

    // Initialize the new sound
    ma_result result = ma_sound_init_from_file(
        &audioEngine, filename, MA_SOUND_FLAG_STREAM, NULL, NULL, fadeInSound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio for crossfade: " << filename
                  << std::endl;
        return false;
    }
    *fadeInInitialized = true;

    // Configure the new sound
    ma_sound_set_looping(fadeInSound, loop ? MA_TRUE : MA_FALSE);
    ma_sound_set_volume(fadeInSound, 0.0f);  // Start silent
    ma_sound_start(fadeInSound);

    // Start crossfade
    audioFile = filename;
    isLooping = loop;
    fadeDuration = duration;
    fadeTimeRemaining = duration;
    isCrossfading = true;
    musicStarted = true;

    return true;
}

void AudioController::updateCrossfade(float deltaTime) {
    if (!isCrossfading || !isInitialized) {
        return;
    }

    fadeTimeRemaining -= deltaTime;

    if (fadeTimeRemaining <= 0.0f) {
        // Crossfade complete
        fadeTimeRemaining = 0.0f;
        isCrossfading = false;

        // Stop and cleanup the old sound
        ma_sound* fadeOutSound = soundAActive ? &soundA : &soundB;
        bool* fadeOutInitialized =
            soundAActive ? &soundAInitialized : &soundBInitialized;

        if (*fadeOutInitialized) {
            ma_sound_stop(fadeOutSound);
            ma_sound_uninit(fadeOutSound);
            *fadeOutInitialized = false;
        }

        // Set the new sound to full volume
        ma_sound* fadeInSound = soundAActive ? &soundB : &soundA;
        ma_sound_set_volume(fadeInSound, volume);

        // Swap active sound
        soundAActive = !soundAActive;
    } else {
        // Interpolate volumes
        float t = 1.0f - (fadeTimeRemaining / fadeDuration);  // 0 to 1

        ma_sound* fadeOutSound = soundAActive ? &soundA : &soundB;
        ma_sound* fadeInSound = soundAActive ? &soundB : &soundA;
        bool fadeOutInitialized =
            soundAActive ? soundAInitialized : soundBInitialized;
        bool fadeInInitialized =
            soundAActive ? soundBInitialized : soundAInitialized;

        if (fadeOutInitialized) {
            ma_sound_set_volume(fadeOutSound, volume * (1.0f - t));
        }
        if (fadeInInitialized) {
            ma_sound_set_volume(fadeInSound, volume * t);
        }
    }
}

void AudioController::deserialize(const nlohmann::json& data) {
    if (!data.is_object()) return;

    if (data.contains("audioType") && data["audioType"].is_string()) {
        std::string typeStr = data["audioType"];
        if (typeStr == "MUSIC") {
            audioType = AudioType::MUSIC;
        } else if (typeStr == "AMBIENCE") {
            audioType = AudioType::AMBIENCE;
        } else if (typeStr == "JUMPSCARE") {
            audioType = AudioType::JUMP_SCARE;
        } else if (typeStr == "WALKING") {
            audioType = AudioType::WALKING;
        } else if (typeStr == "FLASHLIGHT") {
            audioType = AudioType::FLASHLIGHT;
        } else if (typeStr == "PAGE_COLLECT") {
            audioType = AudioType::PAGE_COLLECT;
        } else if (typeStr == "STATIC") {
            audioType = AudioType::STATIC;
        }
    }

    if (data.contains("audioFile") && data["audioFile"].is_string()) {
        std::string filename = data["audioFile"];
        bool loop = data.value("loop", false);
        if (initializeMusic(filename.c_str(), loop)) {
            setVolume(data.value("volume", 1.0f));
        }
    }
}

}  // namespace our