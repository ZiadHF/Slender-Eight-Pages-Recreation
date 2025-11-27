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
        renderer->setStaticParams(player->maxHealth, player->health);
    }
};
}  // namespace our