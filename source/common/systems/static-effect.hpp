#pragma once
#include <iostream>  // For debugging
#include "../components/player.hpp"
#include "../ecs/world.hpp"
#include "forward-renderer.hpp"

namespace our {
class StaticEffectSystem {
    PlayerComponent* player = nullptr;

   public:
    void initialize(World* world) {
        for (auto entity : world->getEntities()) {
            if (auto* p = entity->getComponent<PlayerComponent>()) {
                player = p;
                break;
            }
        }
    }
    void update(World* world, ForwardRenderer* renderer) {
        if (player == nullptr) return;

        
        // DEBUG: Print every 60 frames
        static int debugCounter = 0;
        if (++debugCounter >= 60) {
            debugCounter = 0;
            std::cout << "[StaticEffect] Passing to shader - Dist: " << player->distanceToSlenderman 
                      << ", Angle: " << player->angleToSlenderman 
                      << ", LookTime: " << player->lookTime << std::endl;
        }
        
        // Pass values to renderer for static shader
        renderer->setStaticParams(player->distanceToSlenderman,
                                  player->angleToSlenderman, player->lookTime);
    }
};
}  // namespace our