#pragma once
#include <GLFW/glfw3.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>  // For debugging

#include "../components/player.hpp"
#include "../components/slenderman.hpp"
#include "../ecs/world.hpp"

namespace our {
class SlendermanAISystem {
   public:
    Entity* player = nullptr;
    Entity* slenderman = nullptr;
    float gameTime = 0.0f;  // Track total game time for AI difficulty scaling

    void initialize(World* world) {
        gameTime = 0.0f;  // Reset game time on initialization
        // Initialization logic if needed
        for (auto entity : world->getEntities()) {
            if (entity->getComponent<PlayerComponent>()) player = entity;
            if (entity->getComponent<SlendermanComponent>())
                slenderman = entity;
        }
        if (!player || !slenderman) {
            throw std::runtime_error(
                "SlendermanAISystem initialization failed: Player or "
                "Slenderman entity not found.");
        }
    }

    void update(World* world, float deltaTime, ForwardRenderer* renderer) {
        if (!player || !slenderman) return;  // Safety check

        gameTime += deltaTime;  // Track total game time

        auto* slenderComp = slenderman->getComponent<SlendermanComponent>();
        auto* playerComp = player->getComponent<PlayerComponent>();
        bool isPlayerLooking = false;
        bool isPlayerInProximity = false;
        bool canTeleport = false;
        float proximityMultiplier = 1.0f;

        // Calculate distance using world positions
        glm::vec3 playerPos = glm::vec3(player->getLocalToWorldMatrix()[3]);
        glm::vec3 slenderPos =
            glm::vec3(slenderman->getLocalToWorldMatrix()[3]);
        float distance = glm::length(playerPos - slenderPos);

        playerComp->distanceToSlenderman = distance;

        // Make slender always face the player
        glm::vec3 direction = glm::normalize(playerPos - slenderPos);
        float yaw = atan2(direction.x, direction.z) + glm::half_pi<float>();
        slenderman->localTransform.rotation = glm::vec3(0.0f, yaw, 0.0f);

        // Get frustum from renderer and check if Slenderman is inside it
        const Frustum& frustum = renderer->getFrustum();
        bool isInFrustum = frustum.isSphereInside(slenderPos, 1.0f);

        // Check for isPlayerLooking, isPlayerInProximity
        if (distance < slenderComp->detectionDistance) {
            // If player is near Slenderman but not looking directly at him
            if (distance < slenderComp->closeDistance && !isInFrustum) {
                isPlayerInProximity = true;
                proximityMultiplier = 0.5f;
            } else if (isInFrustum) {
                isPlayerLooking = true;
            }
            playerComp->lookTime += deltaTime;
        } else {
            playerComp->lookTime =
                std::max(0.0f, playerComp->lookTime - deltaTime);
        }

        // Health decrease - scales with lookTime (the longer you look, the more
        // damage)
        float distanceFactor =
            1.0f - (distance / slenderComp->detectionDistance);
        float lookTimeMultiplier =
            1.0f + (playerComp->lookTime * slenderComp->lookTimeFactor *
                    proximityMultiplier);
        playerComp->health =
            std::max(0.0f, playerComp->health -
                               distanceFactor * slenderComp->damageRate *
                                   lookTimeMultiplier * deltaTime);

        // Teleportation logic
        slenderComp->timeSinceLastTeleport += deltaTime;
        if ((!isPlayerLooking && !isPlayerInProximity) &&
            (slenderComp->timeSinceLastTeleport >=
             slenderComp->teleportCooldown)) {
            canTeleport = true;
            slenderComp->timeSinceLastTeleport = 0.0f;
        }

        if (canTeleport) {
            // Get random value
            int randomValue = rand() % (slenderComp->maxAIValue -
                                        slenderComp->minAIValue + 1) +
                              slenderComp->minAIValue;

            // Check if teleportation should occur
            // AI value increases with pages collected and time played
            slenderComp->currentAIValue = std::min(
                slenderComp->startingAIValue + playerComp->collectedPages +
                    static_cast<int>(gameTime / 90.0f),
                slenderComp->maxAIValue);

            // Teleport if randomValue is less than or equal to
            // slenderManAIValue
            if (randomValue <= slenderComp->currentAIValue) {
                // Get distances of spawn points
                std::vector<glm::vec3> sortedSpawnPoints =
                    slenderComp->spawnPoints;
                std::vector<std::pair<glm::vec3, float>> spawnPointDistances;
                for (const auto& spawnPoint : sortedSpawnPoints) {
                    float dist = glm::length(spawnPoint - playerPos);
                    spawnPointDistances.push_back({spawnPoint, dist});
                }

                // Sort spawn points by distance (closest first)
                std::sort(spawnPointDistances.begin(),
                          spawnPointDistances.end(),
                          [](const auto& a, const auto& b) {
                              return a.second < b.second;
                          });

                // Get valid spawns
                std::vector<std::pair<glm::vec3, float>> validSpawns;
                for (const auto& spawnPair : spawnPointDistances) {
                    glm::vec3 spawnPoint = spawnPair.first;
                    float distToPlayer = spawnPair.second;
                    bool inFrustum = frustum.isSphereInside(spawnPoint, 1.0f);

                    if (distToPlayer > slenderComp->detectionDistance ||
                        (distToPlayer > slenderComp->closeDistance &&
                         !inFrustum)) {
                        // Valid spawn point
                        validSpawns.push_back(spawnPair);
                    }
                }

                // Calculate aggressiveness ratio (higher = more aggressive)
                float aggressivenessRatio =
                    static_cast<float>(slenderComp->currentAIValue) /
                    static_cast<float>(slenderComp->maxAIValue);

                int index = -1;
                // Based on aggressiveness, pick spawn point
                if (!validSpawns.empty()) {
                    // aggressiveness 1.0 = index 0 (closest)
                    // aggressiveness 0.0 = last index (farthest)
                    // Add some randomness
                    float randomOffset = (rand() % 100) / 100.0f * 0.3f - 0.15f;
                    float indexFloat =
                        (1.0f - aggressivenessRatio + randomOffset) *
                        (validSpawns.size() - 1);
                    index = std::clamp((int)indexFloat, 0,
                                       (int)validSpawns.size() - 1);

                    slenderman->localTransform.position =
                        validSpawns[index].first;
                }

                // // DEBUG: Print teleportation info
                // std::cout << "=== TELEPORTATION DEBUG ===" << std::endl;
                // std::cout << "[SlenderAI] canTeleport: " << canTeleport
                //           << std::endl;
                // std::cout << "[SlenderAI] timeSinceLastTeleport: "
                //           << slenderComp->timeSinceLastTeleport << std::endl;
                // std::cout << "[SlenderAI] teleportCooldown: "
                //           << slenderComp->teleportCooldown << std::endl;
                // std::cout << "[SlenderAI] isPlayerLooking: " << isPlayerLooking
                //           << std::endl;
                // std::cout << "[SlenderAI] isPlayerInProximity: "
                //           << isPlayerInProximity << std::endl;
                // std::cout << "[SlenderAI] randomValue: " << randomValue
                //           << std::endl;
                // std::cout << "[SlenderAI] currentAIValue: "
                //           << slenderComp->currentAIValue << std::endl;
                // std::cout << "[SlenderAI] aggressivenessRatio: "
                //           << aggressivenessRatio << std::endl;
                // std::cout << "[SlenderAI] Total spawn points: "
                //           << spawnPointDistances.size() << std::endl;
                // std::cout << "[SlenderAI] Valid spawn points: "
                //           << validSpawns.size() << std::endl;

                // for (size_t i = 0; i < validSpawns.size(); i++) {
                //     std::cout << "[SlenderAI] Valid Spawn #" << i << ": ("
                //               << validSpawns[i].first.x << ", "
                //               << validSpawns[i].first.y << ", "
                //               << validSpawns[i].first.z
                //               << ") Dist: " << validSpawns[i].second
                //               << std::endl;
                // }

                // if (!validSpawns.empty()) {
                //     std::cout << "[SlenderAI] Selected index: " << index
                //               << std::endl;
                //     std::cout << "[SlenderAI] Teleporting to: ("
                //               << validSpawns[index].first.x << ", "
                //               << validSpawns[index].first.y << ", "
                //               << validSpawns[index].first.z << ")" << std::endl;
                // }
                // std::cout << "===========================" << std::endl;
            }
        }

        // Health regeneration logic
        if (!isPlayerLooking && !isPlayerInProximity) {
            playerComp->health = std::min(
                playerComp->maxHealth,
                playerComp->health + playerComp->healthRegenRate * deltaTime);
        }
    }
};
}  // namespace our