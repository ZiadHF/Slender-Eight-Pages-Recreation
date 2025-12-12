#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdlib>
#include <cmath>

namespace our
{

    enum class LightType
    {
        DIRECTIONAL = 0,
        POINT = 1,
        SPOT = 2
    };

    class LightComponent : public Component
    {
    public:
        LightType lightType = LightType::POINT;
        glm::vec3 color = glm::vec3(1.0f);
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 attenuation = glm::vec3(1.0f, 0.09f, 0.032f); // constant, linear, quadratic
        float inner_cone_angle = glm::cos(glm::radians(12.5f));
        float outer_cone_angle = glm::cos(glm::radians(17.5f));

        // Whether this is the player's flashlight (can be toggled on/off)
        bool isFlashlight = false;

        // Flickering settings
        bool flickerEnabled = false;
        float flickerIntensity = 0.15f; // How much the light flickers (0-1)
        float flickerSpeed = 8.0f;      // How fast it flickers
        float flickerRandomness = 0.5f; // Mix of smooth vs random flicker

        // Runtime state for flickering
        float flickerTime = 0.0f;
        float currentFlickerValue = 1.0f;
        float targetFlickerValue = 1.0f;
        float flickerLerpSpeed = 10.0f;

        static std::string getID() { return "Light"; }

        // Update flickering - call this each frame with deltaTime
        void updateFlicker(float deltaTime)
        {
            if (!flickerEnabled)
            {
                currentFlickerValue = 1.0f;
                return;
            }

            flickerTime += deltaTime;

            // Smooth sine-based flicker
            float smoothFlicker = 1.0f - flickerIntensity * 0.5f *
                                             (sin(flickerTime * flickerSpeed) * 0.5f +
                                              sin(flickerTime * flickerSpeed * 2.3f) * 0.3f +
                                              sin(flickerTime * flickerSpeed * 5.7f) * 0.2f);

            // Random flicker component (occasional dips)
            static float nextRandomTime = 0.0f;
            if (flickerTime > nextRandomTime)
            {
                float randVal = (float)rand() / RAND_MAX;
                if (randVal < 0.1f)
                { // 10% chance of a dip
                    targetFlickerValue = 1.0f - flickerIntensity * (0.5f + randVal * 5.0f);
                    targetFlickerValue = glm::max(targetFlickerValue, 0.0f);
                }
                else
                {
                    targetFlickerValue = smoothFlicker;
                }
                nextRandomTime = flickerTime + 0.05f + (float)rand() / RAND_MAX * 0.1f;
            }

            // Lerp towards target for smoother transitions
            currentFlickerValue = glm::mix(currentFlickerValue, targetFlickerValue,
                                           glm::min(1.0f, deltaTime * flickerLerpSpeed));

            // Mix smooth and random based on randomness setting
            currentFlickerValue = glm::mix(smoothFlicker, currentFlickerValue, flickerRandomness);
        }

        // Get the effective color with flicker applied
        glm::vec3 getEffectiveColor() const
        {
            return color * currentFlickerValue;
        }

        void deserialize(const nlohmann::json &data) override
        {
            // Parse light type
            std::string typeStr = data.value("lightType", "point");
            if (typeStr == "directional")
            {
                lightType = LightType::DIRECTIONAL;
            }
            else if (typeStr == "spot")
            {
                lightType = LightType::SPOT;
            }
            else
            {
                lightType = LightType::POINT;
            }

            // Parse color
            if (data.contains("color"))
            {
                auto &c = data["color"];
                color = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());
            }

            // Parse direction
            if (data.contains("direction"))
            {
                auto &d = data["direction"];
                direction = glm::normalize(glm::vec3(d[0].get<float>(), d[1].get<float>(), d[2].get<float>()));
            }

            // Parse attenuation (constant, linear, quadratic)
            if (data.contains("attenuation"))
            {
                auto &a = data["attenuation"];
                attenuation = glm::vec3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
            }

            // Parse cone angles for spotlight (in degrees, stored as cosines)
            if (data.contains("innerConeAngle"))
            {
                inner_cone_angle = glm::cos(glm::radians(data["innerConeAngle"].get<float>()));
            }
            if (data.contains("outerConeAngle"))
            {
                outer_cone_angle = glm::cos(glm::radians(data["outerConeAngle"].get<float>()));
            }

            // Parse flicker settings
            std::string flickerModeStr = data.value("flickerMode", "none");
            if (flickerModeStr == "candle" || flickerModeStr == "random" ||
                flickerModeStr == "pulse" || flickerModeStr == "strobe")
            {
                flickerEnabled = true;
            }
            else
            {
                flickerEnabled = data.value("flickerEnabled", false);
            }
            flickerIntensity = data.value("flickerIntensity", 0.15f);
            flickerSpeed = data.value("flickerFrequency", data.value("flickerSpeed", 8.0f));
            flickerRandomness = data.value("flickerRandomness", 0.5f);

            // Parse flashlight flag
            isFlashlight = data.value("isFlashlight", false);
        }
    };

} // namespace our
