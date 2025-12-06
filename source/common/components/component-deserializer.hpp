#pragma once

#include "../ecs/entity.hpp"
#include "audio-controller.hpp"
#include "camera.hpp"
#include "free-camera-controller.hpp"
#include "instanced-renderer.hpp"
#include "mesh-renderer.hpp"
#include "movement.hpp"
#include "../components/player.hpp"
#include "../components/slenderman.hpp"
#include "../components/page.hpp"
#include "../components/page-spawner.hpp"
#include "../components/collider.hpp"

namespace our {

// Given a json object, this function picks and creates a component in the given
// entity based on the "type" specified in the json object which is later
// deserialized from the rest of the json object
inline void deserializeComponent(const nlohmann::json& data, Entity* entity) {
    std::string type = data.value("type", "");
    Component* component = nullptr;
    if (type == CameraComponent::getID()) {
        component = entity->addComponent<CameraComponent>();
    } else if (type == FreeCameraControllerComponent::getID()) {
        component = entity->addComponent<FreeCameraControllerComponent>();
    } else if (type == MovementComponent::getID()) {
        component = entity->addComponent<MovementComponent>();
    } else if (type == MeshRendererComponent::getID()) {
        component = entity->addComponent<MeshRendererComponent>();
    } else if (type == InstancedRendererComponent::getID()) {
        component = entity->addComponent<InstancedRendererComponent>();
    } else if (type == AudioController::getID()) {
        component = entity->addComponent<AudioController>();
    }
    else if (type == our::PlayerComponent::getID()) {
        component = entity->addComponent<our::PlayerComponent>();
    }
    else if (type == our::SlendermanComponent::getID()) {
        component = entity->addComponent<our::SlendermanComponent>();
    }
    else if (type == our::PageSpawnerComponent::getID()) {
        component = entity->addComponent<our::PageSpawnerComponent>();
    }
    else if (type == our::PageComponent::getID()) {
        component = entity->addComponent<our::PageComponent>();
    }else if (type == our::ColliderComponent::getID()){
        component = entity->addComponent<our::ColliderComponent>();
    }

    if (component) component->deserialize(data);
}

}  // namespace our