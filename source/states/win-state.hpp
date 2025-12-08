#pragma once

#include <application.hpp>
#include <material/material.hpp>
#include <mesh/mesh.hpp>
#include <shader/shader.hpp>
#include <texture/texture-utils.hpp>
#include <texture/texture2d.hpp>

#include "../common/components/audio-controller.hpp"
#include "../common/systems/text-renderer.hpp"

class WinState : public our::State {
    our::TexturedMaterial* backgroundMaterial = nullptr;
    our::Mesh* rectangle = nullptr;
    our::TextRenderer* textRenderer = nullptr;
    our::AudioController* audioController = nullptr;

    float time = 0.0f;
    float fadeDuration = 2.0f;
    bool canExit = false;
    int totalPages = 0;

    void onInitialize() override {
        auto& config = getApp()->getConfig()["scene"];
        totalPages = config["world"][2]["components"][1]["totalPages"].get<int>();
        time = 0.0f;
        canExit = false;

        // Create background material
        backgroundMaterial = new our::TexturedMaterial();
        backgroundMaterial->shader = new our::ShaderProgram();
        backgroundMaterial->shader->attach("assets/shaders/textured.vert",
                                           GL_VERTEX_SHADER);
        backgroundMaterial->shader->attach("assets/shaders/textured.frag",
                                           GL_FRAGMENT_SHADER);
        backgroundMaterial->shader->link();

        // Use a dark/black texture or create solid color
        backgroundMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        // Create rectangle mesh
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
            {0, 1, 2, 2, 3, 0});

        // Initialize text renderer
        textRenderer = new our::TextRenderer();

        // Initialize audio
        // audioController = new our::AudioController();
        // if (audioController->initializeMusic("assets/sounds/win.wav", false)) {
        //     audioController->setVolume(0.7f);
        //     audioController->playMusic();
        // }
    }

    void onDraw(double deltaTime) override {
        time += (float)deltaTime;

        // Calculate fade-in alpha
        float alpha = glm::min(time / fadeDuration, 1.0f);

        // Allow exit after fade completes
        if (time > fadeDuration + 1.0f) {
            canExit = true;
        }

        // Get window size
        auto size = getApp()->getFrameBufferSize();
        glm::mat4 projection =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
        glm::mat4 modelMatrix =
            glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        // Draw black background
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (backgroundMaterial && backgroundMaterial->texture) {
            backgroundMaterial->tint = glm::vec4(0.1f, 0.1f, 0.1f, alpha);
            backgroundMaterial->setup();
            backgroundMaterial->shader->set("transform",
                                            projection * modelMatrix);
            rectangle->draw();
        }

        // Render text
        if (textRenderer) {
            // "YOU SURVIVED" text
            std::string winText = "YOU SURVIVED";
            float textScale = 1.5f;
            glm::vec2 textSize = textRenderer->measureText(winText, textScale);
            glm::vec2 winPos =
                glm::vec2((size.x - textSize.x) / 2.0f, size.y / 3.0f);

            glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, alpha);
            textRenderer->renderText(winText, winPos, textScale, textColor,
                                     projection);

            // "All totalPages pages collected" text
            std::string subText = "All " + std::to_string(totalPages) + " pages collected";
            float subScale = 0.6f;
            glm::vec2 subSize = textRenderer->measureText(subText, subScale);
            glm::vec2 subPos =
                glm::vec2((size.x - subSize.x) / 2.0f, size.y / 2.0f);
            textRenderer->renderText(subText, subPos, subScale,
                                     glm::vec4(0.7f, 0.7f, 0.7f, alpha),
                                     projection);

            // "Press any key to continue" text (after can exit)
            if (canExit) {
                std::string continueText = "Press any key to continue";
                float continueScale = 0.4f;
                glm::vec2 continueSize =
                    textRenderer->measureText(continueText, continueScale);
                glm::vec2 continuePos =
                    glm::vec2((size.x - continueSize.x) / 2.0f, size.y * 0.7f);

                // Pulsing effect
                float pulse = (sin(time * 3.0f) + 1.0f) / 2.0f * 0.5f + 0.5f;
                textRenderer->renderText(
                    continueText, continuePos, continueScale,
                    glm::vec4(0.5f, 0.5f, 0.5f, pulse), projection);
            }
        }

        // Check for input to return to menu
        if (canExit) {
            auto& keyboard = getApp()->getKeyboard();
            auto& mouse = getApp()->getMouse();

            // Check for any key press or mouse click
            bool anyInput = mouse.justPressed(GLFW_MOUSE_BUTTON_LEFT) ||
                            keyboard.justPressed(GLFW_KEY_SPACE) ||
                            keyboard.justPressed(GLFW_KEY_ENTER) ||
                            keyboard.justPressed(GLFW_KEY_ESCAPE);

            if (anyInput) {
                getApp()->changeState("menu");
            }
        }
    }

    void onDestroy() override {
        if (backgroundMaterial) {
            if (backgroundMaterial->shader) {
                delete backgroundMaterial->shader;
            }
            if (backgroundMaterial->texture) {
                delete backgroundMaterial->texture;
            }
            delete backgroundMaterial;
            backgroundMaterial = nullptr;
        }

        if (rectangle) {
            delete rectangle;
            rectangle = nullptr;
        }

        if (textRenderer) {
            delete textRenderer;
            textRenderer = nullptr;
        }

        if (audioController) {
            audioController->uninitializeMusic();
            delete audioController;
            audioController = nullptr;
        }
    }
};