#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../ecs/component.hpp"
namespace our {

inline int stringToGLFWKey(const std::string& keyName) {
    static const std::unordered_map<std::string, int> keyMap = {
        // Letters
        {"A", GLFW_KEY_A},
        {"B", GLFW_KEY_B},
        {"C", GLFW_KEY_C},
        {"D", GLFW_KEY_D},
        {"E", GLFW_KEY_E},
        {"F", GLFW_KEY_F},
        {"G", GLFW_KEY_G},
        {"H", GLFW_KEY_H},
        {"I", GLFW_KEY_I},
        {"J", GLFW_KEY_J},
        {"K", GLFW_KEY_K},
        {"L", GLFW_KEY_L},
        {"M", GLFW_KEY_M},
        {"N", GLFW_KEY_N},
        {"O", GLFW_KEY_O},
        {"P", GLFW_KEY_P},
        {"Q", GLFW_KEY_Q},
        {"R", GLFW_KEY_R},
        {"S", GLFW_KEY_S},
        {"T", GLFW_KEY_T},
        {"U", GLFW_KEY_U},
        {"V", GLFW_KEY_V},
        {"W", GLFW_KEY_W},
        {"X", GLFW_KEY_X},
        {"Y", GLFW_KEY_Y},
        {"Z", GLFW_KEY_Z},
        // Numbers
        {"0", GLFW_KEY_0},
        {"1", GLFW_KEY_1},
        {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3},
        {"4", GLFW_KEY_4},
        {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6},
        {"7", GLFW_KEY_7},
        {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9},
        // Arrows
        {"UP", GLFW_KEY_UP},
        {"DOWN", GLFW_KEY_DOWN},
        {"LEFT", GLFW_KEY_LEFT},
        {"RIGHT", GLFW_KEY_RIGHT},
        // Special keys
        {"SPACE", GLFW_KEY_SPACE},
        {"ESCAPE", GLFW_KEY_ESCAPE},
        {"ENTER", GLFW_KEY_ENTER},
        {"TAB", GLFW_KEY_TAB},
        {"BACKSPACE", GLFW_KEY_BACKSPACE},
        {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT},
        {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
        {"LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL},
        {"RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
        {"LEFT_ALT", GLFW_KEY_LEFT_ALT},
        {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT},
        // Mouse buttons
        {"LEFT_CLICK", GLFW_MOUSE_BUTTON_LEFT},
        {"RIGHT_CLICK", GLFW_MOUSE_BUTTON_RIGHT},
        {"MIDDLE_CLICK", GLFW_MOUSE_BUTTON_MIDDLE},
    };

    auto it = keyMap.find(keyName);
    return (it != keyMap.end()) ? it->second : -1;
}

class PlayerComponent : public Component {
   public:
    // Number of pages collected by the player
    int collectedPages = 0;

    // Player health
    float maxHealth = 100.0f;
    float health = maxHealth;
    float healthRegenRate = 5.0f; // Health regeneration rate per second

    // Slenderman interaction
    float lookTime = 0.0f;  // Time the player has been looking at Slenderman
    float distanceToSlenderman = 0.0f;  // Distance to Slenderman

    // Movement speeds
    glm::vec3 walkSpeed = glm::vec3(3.0f);    // Walking speed in units per second
    glm::vec3 sprintSpeedup = glm::vec3(2.0f);  // Sprinting speed in units per second

    // Player states
    bool isMoving = false;
    bool isSprinting = false;

    // Flashlight state
    bool flashlightOn = true;

    static std::string getID() { return "Player Component"; }

    void deserialize(const nlohmann::json& data) override {
        if (!data.is_object()) return;
        // Health parameters
        maxHealth = data.value("maxHealth", maxHealth);
        healthRegenRate = data.value("healthRegenRate", healthRegenRate);

        if (data.contains("walkSpeed")) {
            float speed = data["walkSpeed"];
            walkSpeed = glm::vec3(speed);
        }
        
        if (data.contains("sprintSpeedup")) {
            float speedup = data["sprintSpeedup"];
            sprintSpeedup = glm::vec3(speedup);
        }
    }
};

};  // namespace our