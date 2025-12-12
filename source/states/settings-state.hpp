#pragma once

#include <application.hpp>
#include <array>
#include <functional>
#include <material/material.hpp>
#include <mesh/mesh.hpp>
#include <random>
#include <shader/shader.hpp>
#include <texture/texture-utils.hpp>
#include <texture/texture2d.hpp>
#include <vector>
#include <fstream>
#include <json/json.hpp>
#include <map>
#include "../common/systems/text-renderer.hpp"



namespace our{

    // Map GLFW keys to texture filenames for all visible keys
    std::map<int, std::string> keyToTexture = {
        {GLFW_KEY_ESCAPE, "Escape.png"},
        {GLFW_KEY_F1, "F1.png"},
        {GLFW_KEY_F2, "F2.png"},
        {GLFW_KEY_F3, "F3.png"},
        {GLFW_KEY_F4, "F4.png"},
        {GLFW_KEY_F5, "F5.png"},
        {GLFW_KEY_F6, "F6.png"},
        {GLFW_KEY_F7, "F7.png"},
        {GLFW_KEY_F8, "F8.png"},
        {GLFW_KEY_F9, "F9.png"},
        {GLFW_KEY_F10, "F10.png"},
        {GLFW_KEY_F11, "F11.png"},
        {GLFW_KEY_F12, "F12.png"},
        {GLFW_KEY_1, "1.png"},
        {GLFW_KEY_2, "2.png"},
        {GLFW_KEY_3, "3.png"},
        {GLFW_KEY_4, "4.png"},
        {GLFW_KEY_5, "5.png"},
        {GLFW_KEY_6, "6.png"},
        {GLFW_KEY_7, "7.png"},
        {GLFW_KEY_8, "8.png"},
        {GLFW_KEY_9, "9.png"},
        {GLFW_KEY_0, "0.png"},
        {GLFW_KEY_MINUS, "Hypen_Underscore.png"},
        {GLFW_KEY_EQUAL, "Equal_Plus.png"},
        {GLFW_KEY_BACKSPACE, "Backspace.png"},
        {GLFW_KEY_TAB, "Tab.png"},
        {GLFW_KEY_Q, "Q.png"},
        {GLFW_KEY_W, "W.png"},
        {GLFW_KEY_E, "E.png"},
        {GLFW_KEY_R, "R.png"},
        {GLFW_KEY_T, "T.png"},
        {GLFW_KEY_Y, "Y.png"},
        {GLFW_KEY_U, "U.png"},
        {GLFW_KEY_I, "I.png"},
        {GLFW_KEY_O, "O.png"},
        {GLFW_KEY_P, "P.png"},
        {GLFW_KEY_CAPS_LOCK, "CapsLock.png"},
        {GLFW_KEY_A, "A.png"},
        {GLFW_KEY_S, "S.png"},
        {GLFW_KEY_D, "D.png"},
        {GLFW_KEY_F, "F.png"},
        {GLFW_KEY_G, "G.png"},
        {GLFW_KEY_H, "H.png"},
        {GLFW_KEY_J, "J.png"},
        {GLFW_KEY_K, "K.png"},
        {GLFW_KEY_L, "L.png"},
        {GLFW_KEY_ENTER, "Enter.png"},
        {GLFW_KEY_LEFT_SHIFT, "Shift.png"},
        {GLFW_KEY_Z, "Z.png"},
        {GLFW_KEY_X, "X.png"},
        {GLFW_KEY_C, "C.png"},
        {GLFW_KEY_V, "V.png"},
        {GLFW_KEY_B, "B.png"},
        {GLFW_KEY_N, "N.png"},
        {GLFW_KEY_M, "M.png"},
        {GLFW_KEY_LEFT_CONTROL, "LeftControl.png"},
        {GLFW_KEY_LEFT_ALT, "Alt.png"},
        {GLFW_KEY_SPACE, "Spacebar.png"},
        {GLFW_KEY_RIGHT_ALT, "RightAlt.png"},
        {GLFW_KEY_RIGHT_CONTROL, "Control.png"},
        {GLFW_KEY_LEFT, "LeftArrow.png"},
        {GLFW_KEY_RIGHT, "RightArrow.png"},
        {GLFW_KEY_DOWN, "DownArrow.png"},
        {GLFW_KEY_UP, "UpArrow.png"}};

    // Helper function to get key name from GLFW key code
    std::string getKeyName(int key)
    {
        if (key == GLFW_KEY_W)
            return "W";
        if (key == GLFW_KEY_A)
            return "A";
        if (key == GLFW_KEY_S)
            return "S";
        if (key == GLFW_KEY_D)
            return "D";
        if (key == GLFW_KEY_F)
            return "F";
        if (key == GLFW_KEY_LEFT_SHIFT)
            return "LEFT_SHIFT";
        if (key == GLFW_KEY_ESCAPE)
            return "ESCAPE";
        if (key == GLFW_KEY_SPACE)
            return "SPACE";
        if (key == GLFW_KEY_TAB)
            return "TAB";
        if (key == GLFW_KEY_CAPS_LOCK)
            return "CAPS_LOCK";
        if (key == GLFW_KEY_ENTER)
            return "ENTER";
        if (key == GLFW_KEY_BACKSPACE)
            return "BACKSPACE";
        if (key == GLFW_KEY_LEFT_CONTROL)
            return "LEFT_CONTROL";
        if (key == GLFW_KEY_RIGHT_CONTROL)
            return "RIGHT_CONTROL";
        if (key == GLFW_KEY_LEFT_ALT)
            return "LEFT_ALT";
        if (key == GLFW_KEY_RIGHT_ALT)
            return "RIGHT_ALT";
        if (key == GLFW_KEY_LEFT)
            return "LEFT";
        if (key == GLFW_KEY_RIGHT)
            return "RIGHT";
        if (key == GLFW_KEY_UP)
            return "UP";
        if (key == GLFW_KEY_DOWN)
            return "DOWN";
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
            return std::string(1, '0' + (key - GLFW_KEY_0));
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
            return std::string(1, 'A' + (key - GLFW_KEY_A));
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12)
            return "F" + std::to_string(key - GLFW_KEY_F1 + 1);
        // Add more keys as needed
        return "UNKNOWN";
    }

    // Helper function to get mouse button name
    std::string getMouseButtonName(int button)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            return "LEFT_CLICK";
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            return "RIGHT_CLICK";
        return "UNKNOWN";
    }

    // Helper function to get GLFW key from string
    int getKeyFromString(const std::string &keyStr)
    {
        if (keyStr == "W")
            return GLFW_KEY_W;
        if (keyStr == "A")
            return GLFW_KEY_A;
        if (keyStr == "S")
            return GLFW_KEY_S;
        if (keyStr == "D")
            return GLFW_KEY_D;
        if (keyStr == "F")
            return GLFW_KEY_F;
        if (keyStr == "LEFT_SHIFT")
            return GLFW_KEY_LEFT_SHIFT;
        if (keyStr == "ESCAPE")
            return GLFW_KEY_ESCAPE;
        if (keyStr == "SPACE")
            return GLFW_KEY_SPACE;
        if (keyStr == "TAB")
            return GLFW_KEY_TAB;
        if (keyStr == "CAPS_LOCK")
            return GLFW_KEY_CAPS_LOCK;
        if (keyStr == "ENTER")
            return GLFW_KEY_ENTER;
        if (keyStr == "BACKSPACE")
            return GLFW_KEY_BACKSPACE;
        if (keyStr == "LEFT_CONTROL")
            return GLFW_KEY_LEFT_CONTROL;
        if (keyStr == "RIGHT_CONTROL")
            return GLFW_KEY_RIGHT_CONTROL;
        if (keyStr == "LEFT_ALT")
            return GLFW_KEY_LEFT_ALT;
        if (keyStr == "RIGHT_ALT")
            return GLFW_KEY_RIGHT_ALT;
        if (keyStr == "LEFT")
            return GLFW_KEY_LEFT;
        if (keyStr == "RIGHT")
            return GLFW_KEY_RIGHT;
        if (keyStr == "UP")
            return GLFW_KEY_UP;
        if (keyStr == "DOWN")
            return GLFW_KEY_DOWN;
        // Mouse buttons (return negative values to distinguish from keys)
        if (keyStr == "LEFT_CLICK")
            return -1;
        if (keyStr == "RIGHT_CLICK")
            return -2;
        if (keyStr.length() == 1 && keyStr[0] >= '0' && keyStr[0] <= '9')
            return GLFW_KEY_0 + (keyStr[0] - '0');
        if (keyStr.length() == 1 && keyStr[0] >= 'A' && keyStr[0] <= 'Z')
            return GLFW_KEY_A + (keyStr[0] - 'A');
        if (keyStr[0] == 'F' && keyStr.length() > 1)
        {
            int num = std::stoi(keyStr.substr(1));
            if (num >= 1 && num <= 12)
                return GLFW_KEY_F1 + num - 1;
        }
        return -1000; // Return distinct value for unknown
    }


};
struct KeyboardIcon
{
    glm::vec2 position, size;
    std::string TextureFile;
    bool highlighted;
    glm::mat4 getLocalToWorld(float screenWidth, float screenHeight) const
    {
        return glm::translate(glm::mat4(1.0f),
                              glm::vec3(position.x * screenWidth, position.y * screenHeight, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x * screenWidth, size.y * screenHeight, 1.0f));
    }
};
std::vector<KeyboardIcon> keyboardIcons = {
    {glm::vec2(0.1453f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/Escape.png", false},
    {glm::vec2(0.1969f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F1.png", false},
    {glm::vec2(0.2414f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F2.png", false},
    {glm::vec2(0.2859f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F3.png", false},
    {glm::vec2(0.3305f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F4.png", false},
    {glm::vec2(0.3750f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F5.png", false},
    {glm::vec2(0.4195f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F6.png", false},
    {glm::vec2(0.4641f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F7.png", false},
    {glm::vec2(0.5070f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F8.png", false},
    {glm::vec2(0.5516f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F9.png", false},
    {glm::vec2(0.5961f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F10.png", false},
    {glm::vec2(0.6391f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F11.png", false},
    {glm::vec2(0.6836f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F12.png", false},
    {glm::vec2(0.6836f, 0.125f), glm::vec2(0.0445f, 0.0556f), "assets/textures/Keyboard/F12.png", false},
    {glm::vec2(0.1836f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/1.png", false},
    {glm::vec2(0.2211f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/2.png", false},
    {glm::vec2(0.2586f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/3.png", false},
    {glm::vec2(0.2961f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/4.png", false},
    {glm::vec2(0.3336f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/5.png", false},
    {glm::vec2(0.3711f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/6.png", false},
    {glm::vec2(0.4086f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/7.png", false},
    {glm::vec2(0.4461f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/8.png", false},
    {glm::vec2(0.4836f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/9.png", false},
    {glm::vec2(0.5211f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/0.png", false},
    {glm::vec2(0.5586f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/Hypen_Underscore.png", false},
    {glm::vec2(0.5961f, 0.1944f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/Equal_Plus.png", false},
    {glm::vec2(0.6352f, 0.1931f), glm::vec2(0.0664f, 0.0639f), "assets/textures/Keyboard/Backspace.png", false},
    {glm::vec2(0.1531f, 0.2708f), glm::vec2(0.0508f, 0.0639f), "assets/textures/Keyboard/Tab.png", false},
    {glm::vec2(0.2109f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/Q.png", false},
    {glm::vec2(0.2484f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/W.png", false},
    {glm::vec2(0.2859f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/E.png", false},
    {glm::vec2(0.3234f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/R.png", false},
    {glm::vec2(0.3609f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/T.png", false},
    {glm::vec2(0.3984f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/Y.png", false},
    {glm::vec2(0.4359f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/U.png", false},
    {glm::vec2(0.4734f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/I.png", false},
    {glm::vec2(0.5109f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/O.png", false},
    {glm::vec2(0.5484f, 0.2708f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/P.png", false},
    {glm::vec2(0.1531f, 0.3417f), glm::vec2(0.0508f, 0.0639f), "assets/textures/Keyboard/CapsLock.png", false},
    {glm::vec2(0.2242f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/A.png", false},
    {glm::vec2(0.2617f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/S.png", false},
    {glm::vec2(0.2992f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/D.png", false},
    {glm::vec2(0.3367f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/F.png", false},
    {glm::vec2(0.3742f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/G.png", false},
    {glm::vec2(0.4117f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/H.png", false},
    {glm::vec2(0.4492f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/J.png", false},
    {glm::vec2(0.4867f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/K.png", false},
    {glm::vec2(0.5242f, 0.3417f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/L.png", false},
    {glm::vec2(0.6320f, 0.3417f), glm::vec2(0.0695f, 0.0681f), "assets/textures/Keyboard/Enter.png", false},
    {glm::vec2(0.1531f, 0.4125f), glm::vec2(0.0727f, 0.0708f), "assets/textures/Keyboard/Shift.png", false},
    {glm::vec2(0.2531f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/Z.png", false},
    {glm::vec2(0.2906f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/X.png", false},
    {glm::vec2(0.3281f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/C.png", false},
    {glm::vec2(0.3656f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/V.png", false},
    {glm::vec2(0.4031f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/B.png", false},
    {glm::vec2(0.4406f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/N.png", false},
    {glm::vec2(0.4781f, 0.4125f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/M.png", false},
    {glm::vec2(0.1531f, 0.4931f), glm::vec2(0.0633f, 0.0611f), "assets/textures/Keyboard/LeftControl.png", false},
    {glm::vec2(0.2539f, 0.4903f), glm::vec2(0.0508f, 0.0639f), "assets/textures/Keyboard/Alt.png", false},
    {glm::vec2(0.3086f, 0.4806f), glm::vec2(0.2023f, 0.0778f), "assets/textures/Keyboard/Spacebar.png", false},
    {glm::vec2(0.5141f, 0.4917f), glm::vec2(0.0633f, 0.0625f), "assets/textures/Keyboard/RightAlt.png", false},
    {glm::vec2(0.5813f, 0.4903f), glm::vec2(0.0508f, 0.0639f), "assets/textures/Keyboard/Control.png", false},
    {glm::vec2(0.7438f, 0.4917f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/LeftArrow.png", false},
    {glm::vec2(0.8188f, 0.4917f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/RightArrow.png", false},
    {glm::vec2(0.7813f, 0.4903f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/DownArrow.png", false},
    {glm::vec2(0.7813f, 0.4167f), glm::vec2(0.0352f, 0.0625f), "assets/textures/Keyboard/UpArrow.png", false},
    {glm::vec2(0.8938f, 0.3972f), glm::vec2(0.0852f, 0.1861f), "assets/textures/Keyboard/MouseBG.png", false},
    {glm::vec2(0.8977f, 0.4722f), glm::vec2(0.0773f, 0.1033f), "assets/textures/Keyboard/BaseMouse.png", false},
    {glm::vec2(0.8992f, 0.4042f), glm::vec2(0.0367f, 0.0617f), "assets/textures/Keyboard/LeftClick.png", false},
    {glm::vec2(0.9375f, 0.4042f), glm::vec2(0.0367f, 0.0617f), "assets/textures/Keyboard/RightClick.png", false},
};

struct ControlLabel
{
    std::string displayName;
    std::string configKey;
    glm::vec2 position;
    glm::vec2 size;

    bool isInside(const glm::vec2 &v, float screenWidth, float screenHeight) const
    {
        float x1 = position.x * screenWidth;
        float y1 = position.y * screenHeight;
        float x2 = x1 + size.x * screenWidth;
        float y2 = y1 + size.y * screenHeight;
        return x1 <= v.x && y1 <= v.y && v.x <= x2 && v.y <= y2;
    }

    glm::mat4 getLocalToWorld(float screenWidth, float screenHeight) const
    {
        return glm::translate(glm::mat4(1.0f),
                              glm::vec3(position.x * screenWidth, position.y * screenHeight, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x * screenWidth, size.y * screenHeight, 1.0f));
    }
};

class SettingsState : public our::State
{
    our::TintedMaterial *keyboardRectMat;
    our::TintedMaterial *keyboardRectBackdropMat;
    our::TexturedMaterial *keyboardKeyMat;
    our::TintedMaterial *controlLabelMat;
    our::TintedMaterial *flashMat;
    our::Mesh *rectangle;
    our::Mesh *keyboardBackdrop;
    std::vector<our::Texture2D *> keyboardTextures;
    std::vector<our::Texture2D *> keyboardTexturesRed; // Red textures for non-highlighted keys
    our::Texture2D *background;
    // Text renderer
    our::TextRenderer *textRenderer;

    // Settings data
    nlohmann::json playerConfig;
    std::string configPath = "config/player.json";

    // Current control being rebound
    std::string rebindingControl = "";
    bool isRebinding = false;
    float flashTimer = 0.0f;

    // Control labels for UI
    std::vector<ControlLabel> controlLabels;

 
    bool adjustingSensitivity = false;

    // Load player config
    void loadConfig()
    {
        std::ifstream file(configPath);
        if (file.is_open())
        {
            file >> playerConfig;
            file.close();
           
        }
    }

    // Save player config
    void saveConfig()
    {
        std::ofstream file(configPath);
        if (file.is_open())
        {
            file << playerConfig.dump(4);
            file.close();
        }
    }

    // Highlight key based on current setting
    void updateKeyHighlights()
    {
        // Reset all highlights
        for (auto &icon : keyboardIcons)
        {
            icon.highlighted = false;
        }

        if (playerConfig.contains("controls"))
        {
            auto controls = playerConfig["controls"];

            // Get all control bindings
            for (auto &[key, value] : controls.items())
            {
                std::string keyBinding = value.get<std::string>();

                // Handle mouse buttons
                if (keyBinding == "LEFT_CLICK" || keyBinding == "RIGHT_CLICK")
                {
                    std::string texName = (keyBinding == "LEFT_CLICK") ? "LeftClick.png" : "RightClick.png";
                    for (size_t i = 0; i < keyboardIcons.size(); i++)
                    {
                        if (keyboardIcons[i].TextureFile.find(texName) != std::string::npos)
                        {
                            keyboardIcons[i].highlighted = true;
                            break;
                        }
                    }
                    continue;
                }

                // Handle keyboard keys
                int glfwKey = our::getKeyFromString(keyBinding);

                if (glfwKey != -1000 && our::keyToTexture.find(glfwKey) != our::keyToTexture.end())
                {
                    std::string texName = our::keyToTexture[glfwKey];

                    // Find and highlight the corresponding icon
                    for (size_t i = 0; i < keyboardIcons.size(); i++)
                    {
                        if (keyboardIcons[i].TextureFile.find(texName) != std::string::npos)
                        {
                            keyboardIcons[i].highlighted = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    void onInitialize() override
    {
        // Load config first
        loadConfig();
        background = our::texture_utils::loadImage("assets/textures/Keyboard/scratchy.png");
        // Clear keyboardTextures in case of re-initialization
        keyboardTextures.clear();
        keyboardTexturesRed.clear();

        // Setting up the materials
        keyboardRectMat = new our::TintedMaterial();
        keyboardRectMat->shader = new our::ShaderProgram();
        keyboardRectMat->shader->attach("assets/shaders/tinted.vert",
                                        GL_VERTEX_SHADER);
        keyboardRectMat->shader->attach("assets/shaders/tinted.frag",
                                        GL_FRAGMENT_SHADER);
        keyboardRectMat->shader->link();
        keyboardRectMat->tint = glm::vec4(61.0f / 255.0f, 61.0f / 255.0f, 61.0f / 255.0f, 1.0f);

        keyboardRectBackdropMat = new our::TintedMaterial();
        keyboardRectBackdropMat->shader = new our::ShaderProgram();
        keyboardRectBackdropMat->shader->attach("assets/shaders/tinted.vert",
                                                GL_VERTEX_SHADER);
        keyboardRectBackdropMat->shader->attach("assets/shaders/tinted.frag",
                                                GL_FRAGMENT_SHADER);
        keyboardRectBackdropMat->shader->link();
        keyboardRectBackdropMat->tint = glm::vec4(32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);

        keyboardKeyMat = new our::TexturedMaterial();
        keyboardKeyMat->shader = new our::ShaderProgram();
        keyboardKeyMat->shader->attach("assets/shaders/keyboard.vert", GL_VERTEX_SHADER);
        keyboardKeyMat->shader->attach("assets/shaders/keyboard.frag", GL_FRAGMENT_SHADER);
        keyboardKeyMat->shader->link();
        keyboardKeyMat->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        keyboardKeyMat->pipelineState.blending.enabled = true;
        keyboardKeyMat->pipelineState.blending.equation = GL_FUNC_ADD;
        keyboardKeyMat->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        keyboardKeyMat->pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
        keyboardKeyMat->sampler = new our::Sampler();
        keyboardKeyMat->sampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        keyboardKeyMat->sampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        keyboardKeyMat->sampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        keyboardKeyMat->sampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        keyboardKeyMat->sampler->set(GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
        keyboardKeyMat->sampler->set(GL_TEXTURE_LOD_BIAS, -0.5f);
        // Create material for control labels
        controlLabelMat = new our::TintedMaterial();
        controlLabelMat->shader = new our::ShaderProgram();
        controlLabelMat->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        controlLabelMat->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        controlLabelMat->shader->link();
        controlLabelMat->tint = glm::vec4(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f);

        // Create material for flashing effect
        flashMat = new our::TintedMaterial();
        flashMat->shader = new our::ShaderProgram();
        flashMat->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        flashMat->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        flashMat->shader->link();
        flashMat->pipelineState.blending.enabled = true;
        flashMat->pipelineState.blending.equation = GL_FUNC_ADD;
        flashMat->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        flashMat->pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;

        rectangle = new our::Mesh(
            {
                {{0.0f, 0.0f, 0.0f},
                 {255, 255, 255, 255},
                 {0.0f, 1.0f},
                 {0.0f, 0.0f, 1.0f}},
                {{1.0f, 0.0f, 0.0f},
                 {255, 255, 255, 255},
                 {1.0f, 1.0f},
                 {0.0f, 0.0f, 1.0f}},
                {{1.0f, 1.0f, 0.0f},
                 {255, 255, 255, 255},
                 {1.0f, 0.0f},
                 {0.0f, 0.0f, 1.0f}},
                {{0.0f, 1.0f, 0.0f},
                 {255, 255, 255, 255},
                 {0.0f, 0.0f},
                 {0.0f, 0.0f, 1.0f}},
            },
            {
                0,
                1,
                2,
                2,
                3,
                0,
            });

        keyboardTextures.reserve(keyboardIcons.size());
        keyboardTexturesRed.reserve(keyboardIcons.size());
        for (const auto &icon : keyboardIcons)
        {
            keyboardTextures.push_back(our::texture_utils::loadImage(icon.TextureFile, true));
            // Load red version by replacing "Keyboard/" with "Keyboard/red/"
            std::string redPath = icon.TextureFile;
            size_t pos = redPath.find("Keyboard/");
            if (pos != std::string::npos)
            {
                redPath.replace(pos, 9, "Keyboard/red/");
            }
            keyboardTexturesRed.push_back(our::texture_utils::loadImage(redPath, true));
        }

        // Setup control labels at bottom in 2 columns (normalized coordinates)
        controlLabels = {
            {"Forward", "forward", glm::vec2(0.1563f, 0.6111f), glm::vec2(0.1563f, 0.0694f)},
            {"Backward", "backward", glm::vec2(0.1563f, 0.6875f), glm::vec2(0.1563f, 0.0694f)},
            {"Left", "left", glm::vec2(0.1563f, 0.7639f), glm::vec2(0.1563f, 0.0694f)},
            {"Interact", "interact", glm::vec2(0.1563f, 0.8403f), glm::vec2(0.1563f, 0.0694f)},
            {"Right", "right", glm::vec2(0.4297f, 0.6111f), glm::vec2(0.1563f, 0.0694f)},
            {"Sprint", "sprint", glm::vec2(0.4297f, 0.6875f), glm::vec2(0.1563f, 0.0694f)},
            {"Flashlight", "toggle_flashlight", glm::vec2(0.4297f, 0.7639f), glm::vec2(0.1563f, 0.0694f)},
        };

        updateKeyHighlights();

        // Initialize text renderer
        textRenderer = new our::TextRenderer();
    }

    void onDraw(double deltaTime) override
    {
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto &keyboard = getApp()->getKeyboard();
        auto &mouse = getApp()->getMouse();

        // Update flash timer
        if (isRebinding)
        {
            flashTimer += deltaTime;
        }

        // Handle rebinding
        if (isRebinding)
        {
            // Check for mouse button press
            if (mouse.justPressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                playerConfig["controls"][rebindingControl] = "LEFT_CLICK";
                isRebinding = false;
                rebindingControl = "";
                flashTimer = 0.0f;
                updateKeyHighlights();
                saveConfig();
            }
            else if (mouse.justPressed(GLFW_MOUSE_BUTTON_RIGHT))
            {
                playerConfig["controls"][rebindingControl] = "RIGHT_CLICK";
                isRebinding = false;
                rebindingControl = "";
                flashTimer = 0.0f;
                updateKeyHighlights();
                saveConfig();
            }
            else
            {
                // Check for key press
                for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
                {
                    if (keyboard.justPressed(key))
                    {
                        std::string keyName = our::getKeyName(key);
                        if (keyName != "UNKNOWN")
                        {
                            playerConfig["controls"][rebindingControl] = keyName;
                            isRebinding = false;
                            rebindingControl = "";
                            flashTimer = 0.0f;
                            updateKeyHighlights();
                            saveConfig();
                            break;
                        }
                    }
                }
            }
            // ESC to cancel rebinding
            if (keyboard.justPressed(GLFW_KEY_ESCAPE))
            {
                isRebinding = false;
                rebindingControl = "";
                flashTimer = 0.0f;
            }
        }
        else
        {
            // Click on control labels to rebind
            glm::vec2 mousePosition = mouse.getMousePosition();
            if (mouse.justPressed(0))
            {
                for (auto &label : controlLabels)
                {
                    glm::ivec2 size = getApp()->getFrameBufferSize();
                    if (label.isInside(mousePosition, size.x, size.y))
                    {
                        isRebinding = true;
                        rebindingControl = label.configKey;
                        flashTimer = 0.0f;
                        break;
                    }
                }
            }

            // ESC to go back to menu
            if (keyboard.justPressed(GLFW_KEY_ESCAPE))
            {
                getApp()->changeState("menu");
            }
        }

        glm::ivec2 size = getApp()->getFrameBufferSize();

        glm::mat4 VP =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);

        // BG
        keyboardKeyMat->texture = background;
        keyboardKeyMat->setup();
        keyboardKeyMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f)));
        keyboardKeyMat->shader->set("highlighted", false);
        rectangle->draw();

        keyboardRectBackdropMat->setup();
        keyboardRectBackdropMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(0.1375f * size.x, 0.1111f * size.y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.7242f * size.x, 0.4639f * size.y, 1.0f)));
        rectangle->draw();
        keyboardRectMat->setup();
        keyboardRectMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(0.1445f * size.x, 0.1236f * size.y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.7094f * size.x, 0.4403f * size.y, 1.0f)));
        rectangle->draw();
        
        for (int i = 0; i < keyboardIcons.size(); i++)
        {
            // Use highlighted texture if highlighted, red texture otherwise
            keyboardKeyMat->texture = keyboardIcons[i].highlighted ? keyboardTextures[i] : keyboardTexturesRed[i];
            keyboardKeyMat->setup();
            keyboardKeyMat->shader->set("transform", VP * keyboardIcons[i].getLocalToWorld(size.x, size.y));
            keyboardKeyMat->shader->set("highlighted", keyboardIcons[i].highlighted);
            rectangle->draw();
        }
        glBindSampler(0, 0);
        
        // Draw control labels section
        for (auto &label : controlLabels)
        {
            // Draw label background
            controlLabelMat->setup();
            glm::mat4 labelTransform = label.getLocalToWorld(size.x, size.y);
            controlLabelMat->shader->set("transform", VP * labelTransform);
            rectangle->draw();

            // Draw flashing effect if this control is being rebound
            if (isRebinding && rebindingControl == label.configKey)
            {
                float flashAlpha = (sin(flashTimer * 8.0f) + 1.0f) / 2.0f;
                flashMat->tint = glm::vec4(1.0f, 1.0f, 0.0f, flashAlpha * 0.5f);
                flashMat->setup();
                flashMat->shader->set("transform", VP * labelTransform);
                rectangle->draw();
            }

            // Render text for label name and current binding
            std::string displayText = label.displayName + ": ";
            if (isRebinding && rebindingControl == label.configKey)
            {
                displayText = label.displayName + ": [Press Key]";
            }

            glm::vec2 textPos = glm::vec2(
                (label.position.x + 0.0039f) * size.x,
                (label.position.y + 0.0486f) * size.y);
            glm::vec4 textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

            textRenderer->renderText(displayText, textPos, 0.5f, textColor, VP);

            // Draw keyboard/mouse icon for the current binding
            if (!isRebinding || rebindingControl != label.configKey)
            {
                if (playerConfig.contains("controls") && playerConfig["controls"].contains(label.configKey))
                {
                    std::string currentKey = playerConfig["controls"][label.configKey].get<std::string>();
                    int keyCode = our::getKeyFromString(currentKey);
                    std::string texturePath = "";

                    // Check if it's a mouse button
                    if (currentKey == "LEFT_CLICK")
                    {
                        texturePath = "assets/textures/Keyboard/LeftClick.png";
                    }
                    else if (currentKey == "RIGHT_CLICK")
                    {
                        texturePath = "assets/textures/Keyboard/RightClick.png";
                    }
                    else if (keyCode != -1000 && our::keyToTexture.find(keyCode) != our::keyToTexture.end())
                    {
                        texturePath = "assets/textures/Keyboard/" + our::keyToTexture[keyCode];
                    }

                    if (!texturePath.empty())
                    {
                        // Find or load the texture
                        our::Texture2D *keyTexture = nullptr;
                        for (size_t i = 0; i < keyboardIcons.size(); i++)
                        {
                            if (keyboardIcons[i].TextureFile == texturePath)
                            {
                                keyTexture = keyboardTexturesRed[i];
                                break;
                            }
                        }

                        if (keyTexture)
                        {
                            keyboardKeyMat->texture = keyTexture;
                            keyboardKeyMat->setup();
                            glm::mat4 iconTransform = glm::translate(glm::mat4(1.0f), glm::vec3((label.position.x + 0.1094f) * size.x, (label.position.y + 0.0035f) * size.y, 0.0f)) *
                                                      glm::scale(glm::mat4(1.0f), glm::vec3(0.0352f * size.x, 0.0625f * size.y, 1.0f));
                            keyboardKeyMat->shader->set("transform", VP * iconTransform);
                            keyboardKeyMat->shader->set("highlighted", false);
                            rectangle->draw();
                        }
                    }
                }
            }
        }
      
        // Render ESC instruction at top
        std::string escText = "Press ESC to return to menu";
        glm::vec2 escPos = glm::vec2(0.0f, 0.0278f * size.y);
        textRenderer->renderText(escText, escPos, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), VP);
    }

    void onDestroy() override
    {
        for (auto *tex : keyboardTextures)
        {
            delete tex;
        }
        for (auto *tex : keyboardTexturesRed)
        {
            delete tex;
        }
        if (keyboardKeyMat->sampler)
        {
            delete keyboardKeyMat->sampler;
        }
        delete rectangle;
        delete keyboardRectMat->shader;
        delete keyboardRectMat;
        delete keyboardRectBackdropMat->shader;
        delete keyboardRectBackdropMat;
        delete keyboardKeyMat->shader;
        delete keyboardKeyMat;
        delete controlLabelMat->shader;
        delete controlLabelMat;
        delete flashMat->shader;
        delete flashMat;
        delete background;
        delete textRenderer;
    }
};