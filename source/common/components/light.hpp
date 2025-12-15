#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <cstdlib>

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
        // Cone angles for spotlight (stored as cosines for optimization)
        float inner_cone_angle = glm::cos(glm::radians(12.5f));
        float outer_cone_angle = glm::cos(glm::radians(17.5f));

        // Boolean for toggling flashlight and enabling its texture
        bool isFlashlight = false;

        // Flicker settings
        bool flickerEnabled = false;
        float flickerIntensity = 0.15f; // How much the light dims (0-1)
        float flickerSpeed = 8.0f;      // Base frequency of flicker
        float flickerRandomness = 0.5f; // Mix of smooth vs random (0 = smooth, 1 = random)

        // Internal flicker state
        float flickerTime = 0.0f;
        float currentFlickerValue = 1.0f;
        float targetFlickerValue = 1.0f;
        float nextRandomTime = 0.0f;

        static std::string getID() { return "Light"; }

        // Update flicker state each frame
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
            if (flickerTime > nextRandomTime)
            {
                float randVal = (float)rand() / RAND_MAX;
                if (randVal < 0.1f)
                {
                    // Occasional random dip
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
                                           glm::min(1.0f, deltaTime * 10.0f));

            // Mix smooth and random based on randomness setting
            currentFlickerValue = glm::mix(smoothFlicker, currentFlickerValue, flickerRandomness);
        }

        // Get the effective color (with flicker applied if enabled)
        glm::vec3 getEffectiveColor() const
        {
            return color * currentFlickerValue;
        }

        void deserialize(const nlohmann::json &data) override
        {
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

            if (data.contains("color"))
            {
                auto &c = data["color"];
                color = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());
            }

            if (data.contains("direction"))
            {
                auto &d = data["direction"];
                direction = glm::normalize(glm::vec3(d[0].get<float>(), d[1].get<float>(), d[2].get<float>()));
            }

            
            if (data.contains("attenuation"))
            {
                auto &a = data["attenuation"];
                // Attenuation for light decay in relation with distance, formated as (constant, linear, quadratic)
                attenuation = glm::vec3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
            }

            
            if (data.contains("innerConeAngle"))
            {
                // Storing angles as cosines as an optimization
                inner_cone_angle = glm::cos(glm::radians(data["innerConeAngle"].get<float>()));
            }
            if (data.contains("outerConeAngle"))
            {
                // Storing angles as cosines as an optimization
                outer_cone_angle = glm::cos(glm::radians(data["outerConeAngle"].get<float>()));
            }

            // Flickering parameters set
            flickerEnabled = data.value("flickerEnabled", false);
            flickerIntensity = data.value("flickerIntensity", 0.15f);
            flickerSpeed = data.value("flickerSpeed", 8.0f);
            flickerRandomness = data.value("flickerRandomness", 0.5f);

            
            isFlashlight = data.value("isFlashlight", false);
        }
    };

} // namespace our
