#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "../ecs/component.hpp"

namespace our {
class SlendermanComponent : public Component {
   public:
    // Spawn points for teleportation
    std::vector<glm::vec3> spawnPoints;

    // Random spawn area bounds (min and max corners)
    glm::vec3 spawnAreaMin = glm::vec3(-100.0f, 0.0f, -100.0f);
    glm::vec3 spawnAreaMax = glm::vec3(100.0f, 0.0f, 100.0f);
    float spawnHeight = 1.0f;   // Y position for spawns
    int maxSpawnAttempts = 10;  // Max attempts to find valid spawn

    // Teleportation parameters
    float teleportCooldown = 3.0f;       // seconds
    float timeSinceLastTeleport = 0.0f;  // to track time since last teleport

    // Distance and angle thresholds for player interaction
    float detectionDistance = 20.0f;
    float closeDistance = 5.0f;

    // Health decrease parameters
    float distanceFactor = 0.01f;  // factor for distance-based health decrease
    float lookTimeFactor = 0.5f;   // factor for look time-based health decrease
    float damageRate = 0.5f;  // damage per second when looking at Slenderman

    // The AI works this way:
    // Each teleportCooldown, we generate a random integer between minAIValue
    // and maxAIValue If the generated value is less than or equal to
    // currentAIValue, Slenderman will find a new position to teleport to
    // (within certain constraints) The AI Value is determined by these factors:
    // 1- The constant AI Value given in the jsonc file (startingAIValue)
    // 2- Time passed since the game started (the longer the time, the higher
    // the chance) 3- Number of pages collected by the player (the more pages,
    // the higher the chance)
    int maxAIValue = 20;
    int minAIValue = 0;
    int startingAIValue = 1;
    int currentAIValue = startingAIValue;

    static std::string getID() { return "Slenderman Component"; }
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

        // Distance threshold
        detectionDistance = data.value("detectionDistance", 20.0f);
        closeDistance = data.value("closeDistance", 5.0f);
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
        if (data.contains("startingAIValue") &&
            data["startingAIValue"].is_number()) {
            startingAIValue = data["startingAIValue"];
            currentAIValue = startingAIValue;
        }

        // Spawn area bounds
        if (data.contains("spawnAreaMin") && data["spawnAreaMin"].is_array() &&
            data["spawnAreaMin"].size() == 3) {
            spawnAreaMin =
                glm::vec3(data["spawnAreaMin"][0], data["spawnAreaMin"][1],
                          data["spawnAreaMin"][2]);
        }
        if (data.contains("spawnAreaMax") && data["spawnAreaMax"].is_array() &&
            data["spawnAreaMax"].size() == 3) {
            spawnAreaMax =
                glm::vec3(data["spawnAreaMax"][0], data["spawnAreaMax"][1],
                          data["spawnAreaMax"][2]);
        }
        spawnHeight = data.value("spawnHeight", 1.0f);
        maxSpawnAttempts = data.value("maxSpawnAttempts", 10);
    }
};
}  // namespace our