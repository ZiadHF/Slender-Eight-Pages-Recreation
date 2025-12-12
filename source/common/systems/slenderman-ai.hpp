#pragma once
#include <GLFW/glfw3.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>  // For debugging
#include <random>

#include "../components/player.hpp"
#include "../components/slenderman.hpp"
#include "../ecs/world.hpp"
#include "physics-system.hpp"

namespace our {
class SlendermanAISystem {
   public:
    Entity* player = nullptr;
    Entity* slenderman = nullptr;
    float gameTime = 0.0f;  // Track total game time for AI difficulty scaling

    // Random number generator
    std::mt19937 rng{std::random_device{}()};

    // Helper function to generate random spawn position
    glm::vec3 generateRandomSpawnPosition(SlendermanComponent* slenderComp) {
        std::uniform_real_distribution<float> distX(
            slenderComp->spawnAreaMin.x, slenderComp->spawnAreaMax.x);
        std::uniform_real_distribution<float> distZ(
            slenderComp->spawnAreaMin.z, slenderComp->spawnAreaMax.z);
        return glm::vec3(distX(rng), slenderComp->spawnHeight, distZ(rng));
    }

    // Check if a spawn position is valid (not inside geometry)
    bool isSpawnPositionValid(const glm::vec3& position, PhysicsSystem* physics,
                              float checkRadius = 1.0f) {
        if (!physics || !physics->getWorld()) return true;

        // Cast a ray downward to check if there's ground beneath
        glm::vec3 rayFrom = position + glm::vec3(0, 5.0f, 0);
        glm::vec3 rayTo = position - glm::vec3(0, 1.0f, 0);

        btVector3 btFrom(rayFrom.x, rayFrom.y, rayFrom.z);
        btVector3 btTo(rayTo.x, rayTo.y, rayTo.z);

        btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
        physics->getWorld()->rayTest(btFrom, btTo, rayCallback);

        // If we don't hit anything, position might be outside the map
        if (!rayCallback.hasHit()) return false;

        // Check if spawn point is inside a wall using a sphere check
        // Cast rays in multiple directions to detect if enclosed
        const glm::vec3 directions[] = {glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
                                        glm::vec3(0, 0, 1),
                                        glm::vec3(0, 0, -1)};

        int blockedCount = 0;
        for (const auto& dir : directions) {
            glm::vec3 checkFrom = position + glm::vec3(0, 1.0f, 0);
            glm::vec3 checkTo = checkFrom + dir * checkRadius;

            btVector3 btCheckFrom(checkFrom.x, checkFrom.y, checkFrom.z);
            btVector3 btCheckTo(checkTo.x, checkTo.y, checkTo.z);

            btCollisionWorld::ClosestRayResultCallback checkCallback(
                btCheckFrom, btCheckTo);
            physics->getWorld()->rayTest(btCheckFrom, btCheckTo, checkCallback);

            if (checkCallback.hasHit() &&
                checkCallback.m_closestHitFraction < 0.5f) {
                blockedCount++;
            }
        }

        // If too many directions are blocked, we're probably inside something
        return blockedCount < 3;
    }

    // Check if player has line of sight to a position (no obstacles blocking)
    bool hasLineOfSight(const glm::vec3& fromPos, const glm::vec3& toPos,
                        PhysicsSystem* physics,
                        Entity* ignoreEntity = nullptr) {
        if (!physics || !physics->getWorld()) return true;

        btVector3 btFrom(fromPos.x, fromPos.y, fromPos.z);
        btVector3 btTo(toPos.x, toPos.y, toPos.z);

        btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
        physics->getWorld()->rayTest(btFrom, btTo, rayCallback);

        if (rayCallback.hasHit()) {
            // Check if we hit something other than the ignored entity
            const btCollisionObject* hitObject = rayCallback.m_collisionObject;
            if (hitObject) {
                Entity* hitEntity =
                    static_cast<Entity*>(hitObject->getUserPointer());
                if (hitEntity != ignoreEntity) {
                    return false;  // Line of sight blocked by something else
                }
            }
        }
        return true;  // No obstacles or only hit the target entity
    }

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

    void update(World* world, float deltaTime, ForwardRenderer* renderer,
                PhysicsSystem* physics) {
        
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

        if(slenderComp->debugMode){return;}
        // Get frustum from renderer and check if Slenderman is inside it
        const Frustum& frustum = renderer->getFrustum();
        bool isInFrustum = frustum.isSphereInside(slenderPos, 1.0f);

        // Frustum check alone is not enough. So we also verify line of sight
        // using raycasting
        if (isInFrustum && physics) {
            glm::vec3 rayFrom =
                playerPos + glm::vec3(0, 1.2f, 0);  // Eye height
            glm::vec3 rayTo =
                slenderPos + glm::vec3(0, 1.0f, 0);  // Slenderman center

            // Use helper function to check line of sight
            if (!hasLineOfSight(rayFrom, rayTo, physics, slenderman)) {
                isInFrustum = false;  // Line of sight blocked by wall/obstacle
            }
        }

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
            // Get random value using modern C++ random
            std::uniform_int_distribution<int> dist(slenderComp->minAIValue,
                                                    slenderComp->maxAIValue);
            int randomValue = dist(rng);

            // Check if teleportation should occur
            // AI value increases with pages collected and time played
            slenderComp->currentAIValue = std::min(
                slenderComp->startingAIValue + playerComp->collectedPages +
                    static_cast<int>(gameTime / 90.0f),
                slenderComp->maxAIValue);

            // Teleport if randomValue is less than or equal to currentAIValue
            if (randomValue <= slenderComp->currentAIValue) {
                // Calculate aggressiveness ratio (higher = more aggressive)
                float aggressivenessRatio =
                    static_cast<float>(slenderComp->currentAIValue) /
                    static_cast<float>(slenderComp->maxAIValue);

                // Determine spawn distance based on aggressiveness
                // More aggressive = spawn closer to player
                float minSpawnDist =
                    slenderComp->closeDistance + 2.0f;  // Never spawn too close
                float maxSpawnDist = slenderComp->detectionDistance * 2.0f;
                float targetDist =
                    glm::mix(maxSpawnDist, minSpawnDist, aggressivenessRatio);

                // Add some randomness to target distance
                std::uniform_real_distribution<float> distVariation(0.7f, 1.3f);
                targetDist *= distVariation(rng);

                glm::vec3 newSpawnPos;
                bool foundValidSpawn = false;

                // Try to find a valid random spawn position
                for (int attempt = 0; attempt < slenderComp->maxSpawnAttempts;
                     attempt++) {
                    // Generate random angle around the player
                    std::uniform_real_distribution<float> angleDist(
                        0.0f, glm::two_pi<float>());
                    float angle = angleDist(rng);

                    // Calculate spawn position at target distance from player
                    glm::vec3 candidatePos =
                        playerPos + glm::vec3(cos(angle) * targetDist, 0.0f,
                                              sin(angle) * targetDist);
                    candidatePos.y = slenderComp->spawnHeight;

                    // Clamp to spawn area bounds
                    candidatePos.x =
                        glm::clamp(candidatePos.x, slenderComp->spawnAreaMin.x,
                                   slenderComp->spawnAreaMax.x);
                    candidatePos.z =
                        glm::clamp(candidatePos.z, slenderComp->spawnAreaMin.z,
                                   slenderComp->spawnAreaMax.z);

                    // Check if position is valid (not inside geometry)
                    if (!isSpawnPositionValid(candidatePos, physics)) {
                        continue;
                    }

                    // Check if spawn would be visible to player (don't spawn in
                    // view)
                    bool inFrustum = frustum.isSphereInside(candidatePos, 1.0f);
                    glm::vec3 eyePos = playerPos + glm::vec3(0, 1.2f, 0);
                    bool hasLOS = hasLineOfSight(
                        eyePos, candidatePos + glm::vec3(0, 1.0f, 0), physics,
                        nullptr);

                    // Valid if either outside frustum or no line of sight
                    if (!inFrustum || !hasLOS) {
                        newSpawnPos = candidatePos;
                        foundValidSpawn = true;
                        break;
                    }
                }

                // If found a valid spawn, teleport
                if (foundValidSpawn) {
                    slenderman->localTransform.position = newSpawnPos;
                }
            }
        }

        // Health regeneration logic
        if (!isPlayerLooking && !isPlayerInProximity) {
            playerComp->health = std::min(
                playerComp->maxHealth,
                playerComp->health + playerComp->healthRegenRate * deltaTime);
        }
    }

    bool playerIsDead() const {
        if (!player) return false;
        auto* playerComp = player->getComponent<PlayerComponent>();
        return playerComp && playerComp->health <= 0.0f;
    }
};
}  // namespace our