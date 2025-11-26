#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../ecs/component.hpp"

namespace our {
    class SlendermanComponent : public Component {
        public: 
            enum class State { IDLE, ATTACKING };
            State currentState = State::IDLE;

            // Spawn points for teleportation
            std::vector<glm::vec3> spawnPoints;

            // Teleportation parameters
            float teleportCooldown = 3.0f; // seconds
            float timeSinceLastTeleport = 0.0f; // to track time since last teleport

            // Distance and angle thresholds for player interaction
            float detectionDistance = 20.0f;
            float closeDistance = 5.0f;
            float detectionAngle = 60.0f; // degrees
            float lookAtAngleThreshold = 30.0f; // degrees
            float proximityDeltaTime = 0.5f; // time increment when player is in proximity

            // Health decrease parameters
            float distanceFactor = 0.01f; // factor for distance-based health decrease
            float lookTimeFactor = 0.5f; // factor for look time-based health decrease
            float damageRate = 10.0f; // damage per second when looking at Slenderman

            // The AI works this way:
            // Each teleportCooldown, we generate a random integer between minAIValue and maxAIValue
            // If the generated value is less than or equal to currentAIValue,
            // Slenderman will find a new position to teleport to (within certain constraints)
            // The AI Value is determined by these factors:
            // 1- The constant AI Value given in the jsonc file (startingAIValue)
            // 2- Time passed since the game started (the longer the time, the higher the chance)
            // 3- Number of pages collected by the player (the more pages, the higher the chance)
            int maxAIValue = 20;
            int minAIValue = 0;
            int startingAIValue = 1;
            int currentAIValue = startingAIValue;

            static std::string getID() {
                return "Slenderman Component";
            }
            void deserialize(const nlohmann::json& data) override {
                if (!data.is_object()) return;
                // Spawn points
                if (data.contains("spawnPoints") && data["spawnPoints"].is_array()) {
                    for (const auto& point : data["spawnPoints"]) {
                        if (point.is_array() && point.size() == 3) {
                            glm::vec3 spawnPoint;
                            spawnPoint.x = point[0];
                            spawnPoint.y = point[1];
                            spawnPoint.z = point[2];
                            spawnPoints.push_back(spawnPoint);
                        }
                    }
                }
                // Teleportation parameters
                teleportCooldown = data.value("teleportCooldown", 3.0f);

                // Distance and angle thresholds
                detectionDistance = data.value("detectionDistance", 20.0f);
                closeDistance = data.value("closeDistance", 5.0f);
                detectionAngle = data.value("detectionAngle", 60.0f);
                lookAtAngleThreshold = data.value("lookAtAngleThreshold", 30.0f);
                proximityDeltaTime = data.value("proximityDeltaTime", 0.5f);
                damageRate = data.value("damageRate", 10.0f);

                // Health decrease parameters
                distanceFactor = data.value("distanceFactor", 0.01f);
                lookTimeFactor = data.value("lookTimeFactor", 0.5f);

                // AI parameters
                if (data.contains("maxAIValue") && data["maxAIValue"].is_number()) {
                    maxAIValue = data["maxAIValue"];
                }
                if (data.contains("minAIValue") && data["minAIValue"].is_number()) {
                    minAIValue = data["minAIValue"];
                }
                if (data.contains("startingAIValue") && data["startingAIValue"].is_number()) {
                    startingAIValue = data["startingAIValue"];
                    currentAIValue = startingAIValue;
                }
            }
    };
}