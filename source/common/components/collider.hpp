#pragma once
#include "../ecs/component.hpp"

namespace our
{

    class ColliderComponent : public Component
    {
        public:
        static std::string getID() { return "Collider"; }
        void deserialize(const nlohmann::json& data) override{};

    };
}
