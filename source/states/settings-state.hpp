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
#include "../common/text-renderer.hpp"

struct KeyboardIcon
{
    glm::vec2 position, size;
    std::string TextureFile;
    bool highlighted;
    glm::mat4 getLocalToWorld(float scaleX, float scaleY) const
    {
        return glm::translate(glm::mat4(1.0f),
                              glm::vec3(position.x * scaleX, position.y * scaleY, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x * scaleX, size.y * scaleY, 1.0f));
    }
};
std::vector<KeyboardIcon> keyboardIcons = {
    {glm::vec2(186.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/Escape.png", false},
    {glm::vec2(252.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F1.png", false},
    {glm::vec2(309.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F2.png", false},
    {glm::vec2(366.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F3.png", false},
    {glm::vec2(423.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F4.png", false},
    {glm::vec2(480.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F5.png", false},
    {glm::vec2(537.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F6.png", false},
    {glm::vec2(594.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F7.png", false},
    {glm::vec2(649.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F8.png", false},
    {glm::vec2(706.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F9.png", false},
    {glm::vec2(763.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F10.png", false},
    {glm::vec2(818.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F11.png", false},
    {glm::vec2(875.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F12.png", false},
    {glm::vec2(875.0f, 90.0f), glm::vec2(57.0f, 40.0f), "assets/textures/Keyboard/F12.png", false},
    {glm::vec2(235.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/1.png", false},
    {glm::vec2(283.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/2.png", false},
    {glm::vec2(331.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/3.png", false},
    {glm::vec2(379.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/4.png", false},
    {glm::vec2(427.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/5.png", false},
    {glm::vec2(475.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/6.png", false},
    {glm::vec2(523.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/7.png", false},
    {glm::vec2(571.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/8.png", false},
    {glm::vec2(619.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/9.png", false},
    {glm::vec2(667.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/0.png", false},
    {glm::vec2(715.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/Hypen_Underscore.png", false},
    {glm::vec2(763.0f, 140.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/Equal_Plus.png", false},
    {glm::vec2(813.0f, 139.0f), glm::vec2(85.0f, 46.0f), "assets/textures/Keyboard/Backspace.png", false},
    {glm::vec2(196.0f, 195.0f), glm::vec2(65.0f, 46.0f), "assets/textures/Keyboard/Tab.png", false},
    {glm::vec2(270.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/Q.png", false},
    {glm::vec2(318.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/W.png", false},
    {glm::vec2(366.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/E.png", false},
    {glm::vec2(414.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/R.png", false},
    {glm::vec2(462.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/T.png", false},
    {glm::vec2(510.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/Y.png", false},
    {glm::vec2(558.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/U.png", false},
    {glm::vec2(606.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/I.png", false},
    {glm::vec2(654.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/O.png", false},
    {glm::vec2(702.0f, 195.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/P.png", false},
    {glm::vec2(196.0f, 246.0f), glm::vec2(65.0f, 46.0f), "assets/textures/Keyboard/CapsLock.png", false},
    {glm::vec2(287.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/A.png", false},
    {glm::vec2(335.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/S.png", false},
    {glm::vec2(383.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/D.png", false},
    {glm::vec2(431.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/F.png", false},
    {glm::vec2(479.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/G.png", false},
    {glm::vec2(527.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/H.png", false},
    {glm::vec2(575.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/J.png", false},
    {glm::vec2(623.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/K.png", false},
    {glm::vec2(671.0f, 246.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/L.png", false},
    {glm::vec2(809.0f, 246.0f), glm::vec2(89.0f, 49.0f), "assets/textures/Keyboard/Enter.png", false},
    {glm::vec2(196.0f, 297.0f), glm::vec2(93.0f, 51.0f), "assets/textures/Keyboard/Shift.png", false},
    {glm::vec2(324.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/Z.png", false},
    {glm::vec2(372.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/X.png", false},
    {glm::vec2(420.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/C.png", false},
    {glm::vec2(468.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/V.png", false},
    {glm::vec2(516.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/B.png", false},
    {glm::vec2(564.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/N.png", false},
    {glm::vec2(612.0f, 297.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/M.png", false},
    {glm::vec2(196.0f, 355.0f), glm::vec2(81.0f, 44.0f), "assets/textures/Keyboard/LeftControl.png", false},
    {glm::vec2(325.0f, 353.0f), glm::vec2(65.0f, 46.0f), "assets/textures/Keyboard/Alt.png", false},
    {glm::vec2(395.0f, 346.0f), glm::vec2(259.0f, 56.0f), "assets/textures/Keyboard/Spacebar.png", false},
    {glm::vec2(658.0f, 354.0f), glm::vec2(81.0f, 45.0f), "assets/textures/Keyboard/RightAlt.png", false},
    {glm::vec2(744.0f, 353.0f), glm::vec2(65.0f, 46.0f), "assets/textures/Keyboard/Control.png", false},
    {glm::vec2(952.0f, 354.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/LeftArrow.png", false},
    {glm::vec2(1048.0f, 354.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/RightArrow.png", false},
    {glm::vec2(1000.0f, 353.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/DownArrow.png", false},
    {glm::vec2(1000.0f, 300.0f), glm::vec2(45.0f, 45.0f), "assets/textures/Keyboard/UpArrow.png", false},
    {glm::vec2(1144.0f, 286.0f), glm::vec2(109.0f, 134.0f), "assets/textures/Keyboard/MouseBG.png", false}, // Cannot be chosen just a bg
    {glm::vec2(1149.0f, 340.0f), glm::vec2(99.0f, 74.4f), "assets/textures/Keyboard/BaseMouse.png", false}, // Cannot be chosen just a bg
    {glm::vec2(1151.0f, 291.0f), glm::vec2(47.0f, 44.4f), "assets/textures/Keyboard/LeftClick.png", false},
    {glm::vec2(1200.0f, 291.0f), glm::vec2(47.0f, 44.4f), "assets/textures/Keyboard/RightClick.png", false},

};

struct ControlLabel
{
    std::string displayName;
    std::string configKey;
    glm::vec2 position;
    glm::vec2 size;

    bool isInside(const glm::vec2 &v) const
    {
        return position.x <= v.x && position.y <= v.y &&
               v.x <= position.x + size.x && v.y <= position.y + size.y;
    }

    glm::mat4 getLocalToWorld(float scaleX, float scaleY) const
    {
        return glm::translate(glm::mat4(1.0f),
                              glm::vec3(position.x * scaleX, position.y * scaleY, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x * scaleX, size.y * scaleY, 1.0f));
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

    // Camera sensitivity
    float cameraSensitivity = 1.2f;
    bool adjustingSensitivity = false;

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
            return "LEFT_ARROW";
        if (key == GLFW_KEY_RIGHT)
            return "RIGHT_ARROW";
        if (key == GLFW_KEY_UP)
            return "UP_ARROW";
        if (key == GLFW_KEY_DOWN)
            return "DOWN_ARROW";
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
        if (keyStr == "LEFT_ARROW")
            return GLFW_KEY_LEFT;
        if (keyStr == "RIGHT_ARROW")
            return GLFW_KEY_RIGHT;
        if (keyStr == "UP_ARROW")
            return GLFW_KEY_UP;
        if (keyStr == "DOWN_ARROW")
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

    // Load player config
    void loadConfig()
    {
        std::ifstream file(configPath);
        if (file.is_open())
        {
            file >> playerConfig;
            file.close();
            cameraSensitivity = playerConfig["camera"]["sensitivity"].get<float>();
        }
    }

    // Save player config
    void saveConfig()
    {
        playerConfig["camera"]["sensitivity"] = cameraSensitivity;
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
                int glfwKey = getKeyFromString(keyBinding);

                if (glfwKey != -1000 && keyToTexture.find(glfwKey) != keyToTexture.end())
                {
                    std::string texName = keyToTexture[glfwKey];

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

        // Setup control labels at bottom in 2 columns
        float startY = 440.0f;
        float spacingY = 55.0f;
        float col1X = 200.0f;
        float col2X = 550.0f;
        controlLabels = {
            {"Forward", "forward", glm::vec2(col1X, startY), glm::vec2(200.0f, 50.0f)},
            {"Backward", "backward", glm::vec2(col1X, startY + spacingY), glm::vec2(200.0f, 50.0f)},
            {"Left", "left", glm::vec2(col1X, startY + spacingY * 2), glm::vec2(200.0f, 50.0f)},
            {"Interact", "interact", glm::vec2(col1X, startY + spacingY * 3), glm::vec2(200.0f, 50.0f)},
            {"Right", "right", glm::vec2(col2X, startY), glm::vec2(200.0f, 50.0f)},
            {"Sprint", "sprint", glm::vec2(col2X, startY + spacingY), glm::vec2(200.0f, 50.0f)},
            {"Flashlight", "toggle_flashlight", glm::vec2(col2X, startY + spacingY * 2), glm::vec2(200.0f, 50.0f)},
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
                        std::string keyName = getKeyName(key);
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
            // Handle sensitivity adjustment with arrow keys
            if (keyboard.isPressed(GLFW_KEY_UP))
            {
                cameraSensitivity = std::min(5.0f, cameraSensitivity + 0.01f);
                saveConfig();
            }
            if (keyboard.isPressed(GLFW_KEY_DOWN))
            {
                cameraSensitivity = std::max(0.1f, cameraSensitivity - 0.01f);
                saveConfig();
            }

            // Click on control labels to rebind
            glm::vec2 mousePosition = mouse.getMousePosition();
            if (mouse.justPressed(0))
            {
                for (auto &label : controlLabels)
                {
                    glm::ivec2 size = getApp()->getFrameBufferSize();
                    float scaleX = size.x / 1280.0f;
                    float scaleY = size.y / 720.0f;
                    glm::vec2 scaledPos = glm::vec2(label.position.x * scaleX, label.position.y * scaleY);
                    glm::vec2 scaledSize = glm::vec2(label.size.x * scaleX, label.size.y * scaleY);

                    if (mousePosition.x >= scaledPos.x && mousePosition.x <= scaledPos.x + scaledSize.x &&
                        mousePosition.y >= scaledPos.y && mousePosition.y <= scaledPos.y + scaledSize.y)
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

        float scaleX = size.x / 1280.0f;
        float scaleY = size.y / 720.0f;

        glm::mat4 VP =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);

        glm::mat4 M =
            glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
        // BG
        keyboardKeyMat->texture = background;
        keyboardKeyMat->setup();
        keyboardKeyMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(0 * scaleX, 0 * scaleY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1280.0f * scaleX, 720.0f * scaleY, 1.0f)));
        keyboardKeyMat->shader->set("highlighted", false);
        rectangle->draw();

        keyboardRectBackdropMat->setup();
        keyboardRectBackdropMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(176.0f * scaleX, 80.0f * scaleY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(927.0f * scaleX, 334.0f * scaleY, 1.0f)));
        rectangle->draw();
        keyboardRectMat->setup();
        keyboardRectMat->shader->set("transform", VP * glm::translate(glm::mat4(1.0f), glm::vec3(185.0f * scaleX, 89.0f * scaleY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(908.0f * scaleX, 317.0f * scaleY, 1.0f)));
        rectangle->draw();
        for (int i = 0; i < keyboardIcons.size(); i++)
        {
            // Use highlighted texture if highlighted, red texture otherwise
            keyboardKeyMat->texture = keyboardIcons[i].highlighted ? keyboardTextures[i] : keyboardTexturesRed[i];
            keyboardKeyMat->setup();
            keyboardKeyMat->shader->set("transform", VP * keyboardIcons[i].getLocalToWorld(scaleX, scaleY));
            keyboardKeyMat->shader->set("highlighted", keyboardIcons[i].highlighted);
            rectangle->draw();
        }
        glBindSampler(0, 0);
        // Draw control labels section
        for (auto &label : controlLabels)
        {
            // Draw label background (sized to fit text + icon)
            controlLabelMat->setup();
            glm::mat4 labelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(label.position.x * scaleX, label.position.y * scaleY, 0.0f)) *
                                       glm::scale(glm::mat4(1.0f), glm::vec3(label.size.x * scaleX, label.size.y * scaleY, 1.0f));
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
            else if (playerConfig.contains("controls") && playerConfig["controls"].contains(label.configKey))
            {

                displayText = label.displayName + ": ";
            }

            glm::vec2 textPos = glm::vec2(
                label.position.x + 5.0f,
                label.position.y + 35.0f);
            glm::vec4 textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Black text

            textRenderer->renderText(displayText, textPos, 0.5f, textColor, VP);

            // Draw keyboard/mouse icon for the current binding
            if (!isRebinding || rebindingControl != label.configKey)
            {
                if (playerConfig.contains("controls") && playerConfig["controls"].contains(label.configKey))
                {
                    std::string currentKey = playerConfig["controls"][label.configKey].get<std::string>();
                    int keyCode = getKeyFromString(currentKey);
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
                    else if (keyCode != -1000 && keyToTexture.find(keyCode) != keyToTexture.end())
                    {
                        texturePath = "assets/textures/Keyboard/" + keyToTexture[keyCode];
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
                            glm::mat4 iconTransform = glm::translate(glm::mat4(1.0f), glm::vec3((label.position.x + 140.0f) * scaleX, (label.position.y + 2.5f) * scaleY, 0.0f)) *
                                                      glm::scale(glm::mat4(1.0f), glm::vec3(45.0f * scaleX, 45.0f * scaleY, 1.0f));
                            keyboardKeyMat->shader->set("transform", VP * iconTransform);
                            keyboardKeyMat->shader->set("highlighted", false);
                            rectangle->draw();
                        }
                    }
                }
            }
        }
        

        // Render camera sensitivity text
        std::string sensitivityText = "Camera Sensitivity: " + std::to_string(cameraSensitivity).substr(0, 4);
        std::string instructionText = "(Use UP/DOWN arrows to adjust)";

        glm::vec2 sensPos = glm::vec2(550.0f, 440.0f + 55.0f * 3.5f);
        glm::vec2 instrPos = glm::vec2(550.0f, 440.0f + 55.0f * 3.9f);

        textRenderer->renderText(sensitivityText, sensPos, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), VP);
        textRenderer->renderText(instructionText, instrPos, 0.4f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), VP);

        // Render ESC instruction at top
        std::string escText = "Press ESC to return to menu";
        glm::vec2 escPos = glm::vec2(0.0f, 20.0f);
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