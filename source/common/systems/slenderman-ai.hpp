#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
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
            if (entity->getComponent<PlayerComponent>())
                player = entity;
            if (entity->getComponent<SlendermanComponent>())
                slenderman = entity;
        }
        if (!player || !slenderman) {
            //throw std::runtime_error(
            //    "SlendermanAISystem initialization failed: Player or Slenderman entity not found.");
        }
    }
    void update(World* world, float deltaTime) {
        if (!player || !slenderman) return;  // Safety check

        gameTime += deltaTime;  // Track total game time

        auto* slenderComp = slenderman->getComponent<SlendermanComponent>();
        auto* playerComp = player->getComponent<PlayerComponent>();
        bool isPlayerLooking = false;
        bool isPlayerInProximity = false;
        bool canTeleport = false;

        // Calculate distance using world positions (in case entities have parents)
        glm::vec3 playerPos = glm::vec3(player->getLocalToWorldMatrix()[3]);
        glm::vec3 slenderPos = glm::vec3(slenderman->getLocalToWorldMatrix()[3]);
        float distance = glm::length(playerPos - slenderPos);

        playerComp->distanceToSlenderman = distance;

        // DEBUG: Print every 60 frames (~1 second at 60fps)
        static int debugCounter = 0;
        if (++debugCounter >= 60) {
            debugCounter = 0;
            std::cout << "[SlenderAI] Player: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")" << std::endl;
            std::cout << "[SlenderAI] Slender: (" << slenderPos.x << ", " << slenderPos.y << ", " << slenderPos.z << ")" << std::endl;
            std::cout << "[SlenderAI] Distance: " << distance << ", Angle: " << playerComp->angleToSlenderman << ", LookTime: " << playerComp->lookTime << std::endl;
        }

        // Get angle between player's forward direction and direction to Slenderman
        glm::vec3 playerForward = glm::normalize(
            glm::vec3(glm::rotate(glm::mat4(1.0f),
                                  glm::radians(player->localTransform.rotation.y),
                                  glm::vec3(0, 1, 0)) *
                      glm::vec4(0, 0, -1, 0)));

        glm::vec3 directionToSlender = glm::normalize(slenderPos - playerPos);
        float angle = glm::degrees(acos(glm::clamp(glm::dot(playerForward, directionToSlender), -1.0f, 1.0f)));

        playerComp->angleToSlenderman = angle;

        // Check for isPlayerLooking, isPlayerInProximity
        if (distance < slenderComp->detectionDistance) {
            // If player is near Slenderman but not looking directly at him
            if (distance < slenderComp->closeDistance && angle > slenderComp->detectionAngle){
                isPlayerInProximity = true;
                playerComp->lookTime += slenderComp->proximityDeltaTime * deltaTime;
            }
            else if (angle < slenderComp->lookAtAngleThreshold) {
                isPlayerLooking = true;
                playerComp->lookTime += deltaTime;
            }
        }
        else {
            playerComp->lookTime = std::max(0.0f, playerComp->lookTime - deltaTime);
        }

        // Health decrease - scales with lookTime (the longer you look, the more damage)
        if (isPlayerLooking) {
            // Looking directly: high damage that increases with lookTime
            float distanceFactor = 1.0f - (distance / slenderComp->detectionDistance);
            float lookTimeMultiplier = 1.0f + (playerComp->lookTime * slenderComp->lookTimeFactor);
            playerComp->health -= distanceFactor * slenderComp->damageRate * lookTimeMultiplier * deltaTime;
        } else if (isPlayerInProximity) {
            // Close but not looking: lower damage (proximity only builds lookTime slower)
            float distanceFactor = 1.0f - (distance / slenderComp->detectionDistance);
            float lookTimeMultiplier = 1.0f + (playerComp->lookTime * slenderComp->lookTimeFactor * 0.5f);
            playerComp->health -= distanceFactor * slenderComp->damageRate * 0.5f * lookTimeMultiplier * deltaTime;
        }

        // Teleportation logic
        slenderComp->timeSinceLastTeleport += deltaTime;
        if ((!isPlayerLooking && !isPlayerInProximity) && (slenderComp->timeSinceLastTeleport >= slenderComp->teleportCooldown)) {
            canTeleport = true;
            slenderComp->timeSinceLastTeleport = 0.0f;
        }

        if (canTeleport) {
            // Get random value
            int randomValue = rand() % (slenderComp->maxAIValue - slenderComp->minAIValue + 1) + slenderComp->minAIValue;

            // Check if teleportation should occur
            // AI value increases with pages collected and time played
            slenderComp->currentAIValue = std::min(
                slenderComp->startingAIValue + playerComp->collectedPages + static_cast<int>(gameTime / 90.0f), 
                slenderComp->maxAIValue);

            // Teleport if randomValue is less than or equal to slenderManAIValue
            if (randomValue <= slenderComp->currentAIValue) {
                // Get distances of spawn points
                std::vector<glm::vec3> sortedSpawnPoints = slenderComp->spawnPoints;
                std::vector<std::pair<glm::vec3, float>> spawnPointDistances;
                for (const auto& spawnPoint : sortedSpawnPoints) {
                    float dist = glm::length(spawnPoint - playerPos);
                    spawnPointDistances.push_back({spawnPoint, dist});
                }

                // Sort spawn points based on AI aggressiveness
                // Higher AI value = sort closest first (more aggressive)
                // Lower AI value = sort farthest first (less aggressive)
                float aggressiveness = static_cast<float>(slenderComp->currentAIValue) / static_cast<float>(slenderComp->maxAIValue);
                
                std::sort(spawnPointDistances.begin(), spawnPointDistances.end(),
                          [aggressiveness](const std::pair<glm::vec3, float>& a,
                             const std::pair<glm::vec3, float>& b) {
                              // aggressiveness > 0.5 = prefer closer, < 0.5 = prefer farther
                              if (aggressiveness > 0.5f) {
                                  return a.second < b.second;  // Closest first
                              } else {
                                  return a.second > b.second;  // Farthest first
                              }
                          });

                // Check if point is valid (not close to player and not within line of sight)
                for (const auto& spawnPair : spawnPointDistances) {
                    glm::vec3 spawnPoint = spawnPair.first;
                    float distToPlayer = spawnPair.second;
                    float angleToPlayer = glm::degrees(acos(glm::clamp(glm::dot(playerForward, glm::normalize(spawnPoint - playerPos)), -1.0f, 1.0f)));

                    if (distToPlayer > slenderComp->detectionDistance || (distToPlayer > slenderComp->closeDistance && angleToPlayer > slenderComp->detectionAngle)) {
                        // Teleport Slenderman to this spawn point
                        slenderman->localTransform.position = spawnPoint;
                        break;
                    }
                }
            }
        }

        // Health regeneration logic
        if (!isPlayerLooking && !isPlayerInProximity) {
            playerComp->health = std::min(playerComp->maxHealth, playerComp->health + playerComp->healthRegenRate * deltaTime);
        }
    }
};
}  // namespace our