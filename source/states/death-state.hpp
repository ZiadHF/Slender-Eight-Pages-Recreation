#pragma once

#include <GLFW/glfw3.h>

#include <application.hpp>
#include <asset-loader.hpp>
#include <iostream>
#include <material/material.hpp>
#include <mesh/mesh-utils.hpp>
#include <mesh/mesh.hpp>
#include <random>
#include <shader/shader.hpp>
#include <texture/texture-utils.hpp>
#include <texture/texture2d.hpp>

#include "../common/components/audio-controller.hpp"
#include "../common/systems/text-renderer.hpp"

class DeathState : public our::State {
    // Postprocessing resources
    GLuint postprocessFrameBuffer = 0;
    GLuint postProcessVertexArray = 0;
    our::Texture2D* colorTarget = nullptr;
    our::Texture2D* depthTarget = nullptr;
    our::ShaderProgram* postprocessShader = nullptr;
    our::Sampler* postprocessSampler = nullptr;

    // Materials for rendering
    our::TexturedMaterial* slendermanMaterial = nullptr;
    our::Mesh* slendermanMesh = nullptr;
    our::TextRenderer* textRenderer = nullptr;
    our::AudioController* audioController = nullptr;

    // Timing
    float time = 0.0f;
    float flickerStartTime = 0.4f;
    float flickerEndTime = 2.4f;
    float fadeEndTime = 5.0f;
    bool canExit = false;
    bool soundOn = false;

    // Random for effects
    std::mt19937 rng{std::random_device{}()};

    void onInitialize() override {
        time = 0.0f;
        canExit = false;
        soundOn = false;

        // Load assets from death config
        auto& config = getApp()->getConfig()["death"];
        if (config.contains("assets")) {
            our::deserializeAllAssets(config["assets"]);
        }

        // Setup postprocessing framebuffer
        auto size = getApp()->getFrameBufferSize();

        glGenFramebuffers(1, &postprocessFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

        colorTarget = our::texture_utils::empty(GL_RGBA8, size);
        depthTarget = our::texture_utils::empty(GL_DEPTH_COMPONENT32F, size);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenVertexArrays(1, &postProcessVertexArray);

        postprocessSampler = new our::Sampler();
        postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        postprocessShader = new our::ShaderProgram();
        postprocessShader->attach("assets/shaders/fullscreen.vert",
                                  GL_VERTEX_SHADER);
        postprocessShader->attach("assets/shaders/postprocess/static.frag",
                                  GL_FRAGMENT_SHADER);
        postprocessShader->link();

        // Get Slenderman material from AssetLoader
        slendermanMaterial = dynamic_cast<our::TexturedMaterial*>(
            our::AssetLoader<our::Material>::get("Slenderman"));
        if (!slendermanMaterial) {
            std::cerr << "Failed to get Slenderman material from AssetLoader!"
                      << std::endl;
        }

        slendermanMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
        slendermanMaterial->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        slendermanMaterial->pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
        slendermanMaterial->pipelineState.blending.enabled = true;
        slendermanMaterial->pipelineState.depthTesting.enabled = true;

        // Get Slenderman mesh from AssetLoader
        slendermanMesh = our::AssetLoader<our::Mesh>::get("slender");
        if (!slendermanMesh) {
            std::cerr << "Failed to get slender mesh from AssetLoader!"
                      << std::endl;
        }

        // Initialize text renderer
        textRenderer = new our::TextRenderer();

        // Initialize death audio
        audioController = new our::AudioController();
        if (audioController->initializeMusic("assets/sounds/static_heavy.wav",
                                             true)) {
            audioController->setVolume(1.0f);
            audioController->playMusic();
        }
    }

    void onDraw(double deltaTime) override {
        time += (float)deltaTime;

        // Sound flickering logic
        if (time < flickerStartTime || time > flickerEndTime) {
            soundOn = false;
        } else {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            soundOn = (dist(rng) < 0.7f);
        }

        // Apply sound state
        if (audioController) {
            if (soundOn) {
                audioController->setVolume(1.0f);
            } else {
                audioController->setVolume(0.0f);
            }
            if (time >= flickerEndTime) {
                audioController->setVolume(0.0f);
            }
        }

        // Get window size
        auto size = getApp()->getFrameBufferSize();
        float aspectRatio = (float)size.x / (float)size.y;

        // Projections
        glm::mat4 orthoProjection =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
        glm::mat4 perspProjection =
            glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);

        // Calculate phase
        bool showSlenderman = false;
        float staticIntensity = 1.0f;  // 0 = no static, 1 = full static

        if (time < flickerStartTime) {
            // Phase 1: Initial static burst
            showSlenderman = false;
            staticIntensity = 1.0f;
        } else if (time < flickerEndTime) {
            // Phase 2: Flickering static + Slenderman
            showSlenderman = soundOn;
            staticIntensity = 1.0f;
        } else if (time < fadeEndTime) {
            // Phase 3: Fade to black
            float fadeProgress =
                (time - flickerEndTime) / (fadeEndTime - flickerEndTime);
            showSlenderman = false;
            staticIntensity = 1.0f - fadeProgress;
        } else {
            // Phase 4: Black screen with text
            showSlenderman = false;
            staticIntensity = 0.0f;
            canExit = true;
        }

        // === RENDER BLACK TO FRAMEBUFFER (for static effect base) ===
        glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        glViewport(0, 0, size.x, size.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Just clear to black - the static shader will generate static over
        // this

        // === APPLY POSTPROCESSING (STATIC) TO SCREEN ===
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, size.x, size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        postprocessShader->use();

        // Bind the color target texture
        glActiveTexture(GL_TEXTURE0);
        colorTarget->bind();
        postprocessSampler->bind(0);
        postprocessShader->set("tex", 0);

        // Set static uniforms - health 0 = full static, health 100 = no static
        float health = 100.0f * (1.0f - staticIntensity);
        postprocessShader->set("health", health);
        postprocessShader->set("maxHealth", 100.0f);
        postprocessShader->set("time", (float)glfwGetTime());

        // Draw fullscreen triangle (renders static)
        glBindVertexArray(postProcessVertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // === DRAW SLENDERMAN ON TOP OF STATIC ===
        if (showSlenderman && slendermanMesh && slendermanMaterial) {
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);  // Clear depth only so Slenderman
                                           // draws on top

            float modelScale = 1.0f;

            // Add shake effect
            float shakeX = 0.0f, shakeY = 0.0f;
            if (soundOn) {
                float shakeAmount = 0.02f;
                shakeX = (rng() % 100 - 50) / 50.0f * shakeAmount;
                shakeY = (rng() % 100 - 50) / 50.0f * shakeAmount;
            }

            // Model matrix
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(
                modelMatrix, glm::vec3(shakeX, -2.4f + shakeY, -0.5f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(modelScale));

            glm::mat4 viewMatrix = glm::mat4(1.0f);
            glm::mat4 MVP = perspProjection * viewMatrix * modelMatrix;

            slendermanMaterial->setup();
            slendermanMaterial->shader->set("transform", MVP);
            slendermanMesh->draw();
        }

        // Render "GAME OVER" text after fade
        if (canExit && textRenderer) {
            glDisable(GL_DEPTH_TEST);

            std::string gameOverText = "GAME OVER";
            float textScale = 1.5f;
            glm::vec2 textSize =
                textRenderer->measureText(gameOverText, textScale);
            glm::vec2 textPos =
                glm::vec2((size.x - textSize.x) / 2.0f, size.y / 3.0f);

            float textAlpha = glm::min((time - fadeEndTime) / 0.5f, 1.0f);
            textRenderer->renderText(gameOverText, textPos, textScale,
                                     glm::vec4(0.8f, 0.0f, 0.0f, textAlpha),
                                     orthoProjection);

            std::string continueText = "Press any key to continue";
            float continueScale = 0.4f;
            glm::vec2 continueSize =
                textRenderer->measureText(continueText, continueScale);
            glm::vec2 continuePos =
                glm::vec2((size.x - continueSize.x) / 2.0f, size.y * 0.6f);

            float pulse = (sin(time * 3.0f) + 1.0f) / 2.0f * 0.5f + 0.5f;
            textRenderer->renderText(
                continueText, continuePos, continueScale,
                glm::vec4(0.5f, 0.5f, 0.5f, pulse * textAlpha),
                orthoProjection);
        }

        // Check for input to return to menu
        if (canExit) {
            auto& keyboard = getApp()->getKeyboard();
            auto& mouse = getApp()->getMouse();

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
        // Cleanup postprocessing resources
        if (postprocessFrameBuffer) {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            postprocessFrameBuffer = 0;
        }
        if (postProcessVertexArray) {
            glDeleteVertexArrays(1, &postProcessVertexArray);
            postProcessVertexArray = 0;
        }
        if (colorTarget) {
            delete colorTarget;
            colorTarget = nullptr;
        }
        if (depthTarget) {
            delete depthTarget;
            depthTarget = nullptr;
        }
        if (postprocessShader) {
            delete postprocessShader;
            postprocessShader = nullptr;
        }
        if (postprocessSampler) {
            delete postprocessSampler;
            postprocessSampler = nullptr;
        }

        slendermanMaterial = nullptr;
        slendermanMesh = nullptr;

        if (textRenderer) {
            delete textRenderer;
            textRenderer = nullptr;
        }

        if (audioController) {
            audioController->uninitializeMusic();
            delete audioController;
            audioController = nullptr;
        }

        our::clearAllAssets();
    }
};