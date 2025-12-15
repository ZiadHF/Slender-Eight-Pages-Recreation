#pragma once

#include <application.hpp>
#include <asset-loader.hpp>
#include <components/camera.hpp>
#include <ecs/world.hpp>
#include <systems/ambient-tension-system.hpp>
#include <systems/footstep-system.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/page-system.hpp>
#include <systems/physics-system.hpp>
#include <systems/slenderman-ai.hpp>
#include <systems/static-effect.hpp>
#include <systems/static-sound-system.hpp>
#include <fstream>
#include <json/json.hpp>

#include "../common/systems/text-renderer.hpp"

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State {
    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::SlendermanAISystem slendermanAISystem;
    our::StaticEffectSystem staticEffectSystem;
    our::PhysicsSystem physicsSystem;
    our::PageSystem pageSystem;
    our::FootstepSystem footstepSystem;
    our::AmbientTensionSystem ambientTensionSystem;
    our::StaticSoundSystem staticSoundSystem;
    our::TextRenderer* textRenderer = nullptr;
    our::TexturedMaterial* loadingMaterial = nullptr;
    our::TintedMaterial* loadingBarMaterial = nullptr;
    our::Mesh* loadingRectangle = nullptr;
    our::Texture2D* scratchyTexture = nullptr;
    
    // Pre-loaded control icons for loading screen
    struct ControlIcon {
        std::string displayName;
        our::Texture2D* texture = nullptr;
    };
    std::vector<ControlIcon> controlIcons;
    
    enum class LoadingStage {
        NOT_STARTED,
        SHOW_BLACK,           // Just show black screen first frame
        INIT_LOADING_UI,      // Initialize loading UI resources
        LOADING_ASSETS,
        LOADING_WORLD,
        INITIALIZING_RENDERER,
        INITIALIZING_SYSTEMS,
        COMPLETE_SPACE,
        COMPLETE
    };
    
    LoadingStage loadingStage = LoadingStage::NOT_STARTED;
    float loadingProgress = 0.0f;
    bool paused = false;
   
    // Helper function to load player config
    nlohmann::json loadPlayerConfig() {
        nlohmann::json config;
        std::ifstream file("config/player.json");
        if (file.is_open()) {
            file >> config;
            file.close();
        }
        return config;
    }

    void onInitialize() override {
        // Reset pause state
        paused = false;
        loadingStage = LoadingStage::NOT_STARTED;
        loadingProgress = 0.0f;
        textRenderer = nullptr;
        loadingMaterial = nullptr;
        loadingBarMaterial = nullptr;
        loadingRectangle = nullptr;
        scratchyTexture = nullptr;
        controlIcons.clear();
    }

    void initializeLoadingResources() {
        // Create material for loading screen
        loadingMaterial = new our::TexturedMaterial();
        loadingMaterial->shader = new our::ShaderProgram();
        loadingMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        loadingMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        loadingMaterial->shader->link();
        loadingMaterial->pipelineState.blending.enabled = true;
        loadingMaterial->pipelineState.blending.equation = GL_FUNC_ADD;
        loadingMaterial->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        loadingMaterial->pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;

        // Create material for loading bar
        loadingBarMaterial = new our::TintedMaterial();
        loadingBarMaterial->shader = new our::ShaderProgram();
        loadingBarMaterial->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        loadingBarMaterial->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        loadingBarMaterial->shader->link();
        loadingBarMaterial->pipelineState.blending.enabled = true;
        loadingBarMaterial->pipelineState.blending.equation = GL_FUNC_ADD;
        loadingBarMaterial->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        loadingBarMaterial->pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;

        // Create rectangle mesh
        loadingRectangle = new our::Mesh(
            {
                {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            },
            {0, 1, 2, 2, 3, 0});
        
        // Pre-load the scratchy texture once
        scratchyTexture = our::texture_utils::loadImage("assets/textures/Keyboard/scratchy.png");
        
        // Initialize text renderer for loading screen
        textRenderer = new our::TextRenderer();
        
        // Pre-load control icons
        loadControlIcons();
    }

    void loadControlIcons() {
        nlohmann::json playerConfig = loadPlayerConfig();
        
        std::vector<std::pair<std::string, std::string>> controls = {
            {"Move Forward", "forward"},
            {"Move Backward", "backward"},
            {"Move Left", "left"},
            {"Move Right", "right"},
            {"Sprint", "sprint"},
            {"Flashlight", "toggle_flashlight"},
            {"Interact", "interact"}
        };

        for (const auto& [displayName, configKey] : controls) {
            ControlIcon icon;
            icon.displayName = displayName;
            icon.texture = nullptr;

            if (playerConfig.contains("controls") && playerConfig["controls"].contains(configKey)) {
                std::string keyBinding = playerConfig["controls"][configKey].get<std::string>();
                int keyCode = our::getKeyFromString(keyBinding);
                std::string texturePath = "";

                if (keyBinding == "LEFT_CLICK") {
                    texturePath = "assets/textures/Keyboard/LeftClick.png";
                } else if (keyBinding == "RIGHT_CLICK") {
                    texturePath = "assets/textures/Keyboard/RightClick.png";
                } else if (keyCode != -1000 && our::keyToTexture.find(keyCode) != our::keyToTexture.end()) {
                    texturePath = "assets/textures/Keyboard/red/" + our::keyToTexture[keyCode];
                }

                if (!texturePath.empty()) {
                    icon.texture = our::texture_utils::loadImage(texturePath, true);
                }
            }
            controlIcons.push_back(icon);
        }
    }

    void performLoadingStep() {
        auto& config = getApp()->getConfig()["scene"];
        auto size = getApp()->getFrameBufferSize();

        switch (loadingStage) {
            case LoadingStage::LOADING_ASSETS:
                if (config.contains("assets")) {
                    our::deserializeAllAssets(config["assets"]);
                }
                loadingProgress = 0.25f;
                loadingStage = LoadingStage::LOADING_WORLD;
                break;
                
            case LoadingStage::LOADING_WORLD:
                if (config.contains("world")) {
                    world.deserialize(config["world"]);
                }
                loadingProgress = 0.50f;
                loadingStage = LoadingStage::INITIALIZING_RENDERER;
                break;
                
            case LoadingStage::INITIALIZING_RENDERER:
                renderer.initialize(size, config["renderer"]);
                physicsSystem.initialize(&world);
                loadingProgress = 0.75f;
                loadingStage = LoadingStage::INITIALIZING_SYSTEMS;
                break;
                
            case LoadingStage::INITIALIZING_SYSTEMS:
                cameraController.enter(getApp(), &physicsSystem);
                slendermanAISystem.initialize(&world);
                staticEffectSystem.initialize(&world);
                pageSystem.initialize(&world, &physicsSystem, textRenderer, glm::vec2(size.x, size.y));
                footstepSystem.initialize(&world, &physicsSystem);
                ambientTensionSystem.initialize(&world);
                staticSoundSystem.initialize(&world);
                
                {
                    glm::vec2 centerPos = glm::vec2(size.x / 2.0f - 75, size.y / 2.0f);
                    textRenderer->startTimedText("Collect " + std::to_string(pageSystem.totalPages) + " Pages", 
                                               15.0f, centerPos, 0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                
                loadingProgress = 1.0f;
                loadingStage = LoadingStage::COMPLETE_SPACE;
                break;
                
            default:
                break;
        }
    }

    void cleanupLoadingResources() {
        if (scratchyTexture) {
            delete scratchyTexture;
            scratchyTexture = nullptr;
        }
        for (auto& icon : controlIcons) {
            if (icon.texture) {
                delete icon.texture;
                icon.texture = nullptr;
            }
        }
        controlIcons.clear();
    }

    void drawLoadingScreen() {
        auto size = getApp()->getFrameBufferSize();
        glm::mat4 VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        // Scale factors for resolution independence (base resolution 1280x720)
        float scaleX = size.x / 1280.0f;
        float scaleY = size.y / 720.0f;
        float scale = std::min(scaleX, scaleY);  // Use min to maintain aspect ratio

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw scratchy background
        if (scratchyTexture && loadingMaterial && loadingRectangle) {
            loadingMaterial->texture = scratchyTexture;
            loadingMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            loadingMaterial->setup();
            loadingMaterial->shader->set("transform", VP * M);
            loadingRectangle->draw();
        }

        // Draw semi-transparent overlay for better text readability
        if (loadingBarMaterial && loadingRectangle) {
            loadingBarMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.4f);
            loadingBarMaterial->setup();
            loadingBarMaterial->shader->set("transform", VP * M);
            loadingRectangle->draw();
        }

        // ========== CONTROLS SECTION ==========
        if (textRenderer && loadingMaterial && loadingRectangle) {
            // Title for controls
            std::string controlsTitle = "CONTROLS";
            float titleScale = 0.8f * scale;
            glm::vec2 titleSize = textRenderer->measureText(controlsTitle, titleScale);
            float controlsStartX = 60.0f * scaleX;
            float controlsStartY = 80.0f * scaleY;
            
            textRenderer->renderText(controlsTitle, 
                glm::vec2(controlsStartX, controlsStartY), 
                titleScale, 
                glm::vec4(1.0f, 0.3f, 0.3f, 1.0f),  // Red title
                VP);

            // Draw underline for title
            if (loadingBarMaterial) {
                loadingBarMaterial->tint = glm::vec4(1.0f, 0.3f, 0.3f, 0.8f);
                loadingBarMaterial->setup();
                glm::mat4 underlineM = glm::translate(glm::mat4(1.0f), 
                    glm::vec3(controlsStartX, controlsStartY + titleSize.y - 15.0f * scale, 0.0f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(titleSize.x + 20.0f * scale, 3.0f * scale, 1.0f));
                loadingBarMaterial->shader->set("transform", VP * underlineM);
                loadingRectangle->draw();
            }

            // Draw controls in two columns
            float iconSize = 45.0f * scale;
            float rowHeight = 55.0f * scale;
            float columnWidth = 280.0f * scaleX;
            float textScale = 0.45f * scale;
            float startY = controlsStartY + 50.0f * scale;
            
            int itemsPerColumn = (controlIcons.size() + 1) / 2;  // Ceiling division

            for (size_t i = 0; i < controlIcons.size(); i++) {
                int column = i / itemsPerColumn;
                int row = i % itemsPerColumn;
                
                float xPos = controlsStartX + column * columnWidth;
                float yPos = startY + row * rowHeight;

                // Draw key icon
                if (controlIcons[i].texture) {
                    glBindSampler(0, 0);  // Unbind sampler before rendering
                    loadingMaterial->texture = controlIcons[i].texture;
                    loadingMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    loadingMaterial->setup();

                    glm::mat4 iconM = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, 0.0f)) *
                                     glm::scale(glm::mat4(1.0f), glm::vec3(iconSize, iconSize, 1.0f));
                    loadingMaterial->shader->set("transform", VP * iconM);
                    loadingRectangle->draw();
                }

                // Draw control name next to icon
                glm::vec2 textPos = glm::vec2(xPos + iconSize + 15.0f * scale, yPos + iconSize / 2.0f + 8.0f * scale);
                textRenderer->renderText(controlIcons[i].displayName, textPos, textScale, 
                    glm::vec4(1.0f, 1.0f, 1.0f, 0.9f), VP);
            }
        }

        // ========== LOADING BAR SECTION (Bottom Right) ==========
        if (textRenderer && loadingBarMaterial && loadingRectangle) {
            float padding = 30.0f * scale;
            float barWidth = 350.0f * scale;
            float barHeight = 25.0f * scale;
            float barRadius = 4.0f * scale;
            
            
            glm::vec2 barPos = glm::vec2(size.x - barWidth - padding, size.y - padding - barHeight);
            
            
            std::string loadingText = (loadingStage == LoadingStage::COMPLETE_SPACE) ? "Ready" : "Loading...";
            float loadingTextScale = 0.6f * scale;
            glm::vec2 loadTextSize = textRenderer->measureText(loadingText, loadingTextScale);
            glm::vec2 textPosition = glm::vec2(barPos.x, barPos.y - 12.0f * scale);
            
          
            textRenderer->renderText(loadingText, textPosition, loadingTextScale, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), VP);
            
            // Draw loading bar background (dark with border effect)
            loadingBarMaterial->tint = glm::vec4(0.15f, 0.15f, 0.15f, 0.9f);
            loadingBarMaterial->setup();
            glm::mat4 barBgM = glm::translate(glm::mat4(1.0f), glm::vec3(barPos, 0.0f)) *
                               glm::scale(glm::mat4(1.0f), glm::vec3(barWidth, barHeight, 1.0f));
            loadingBarMaterial->shader->set("transform", VP * barBgM);
            loadingRectangle->draw();

            // Draw loading bar border
            float borderWidth = 2.0f * scale;
            loadingBarMaterial->tint = glm::vec4(0.5f, 0.5f, 0.5f, 0.8f);
            loadingBarMaterial->setup();
            
            // Top border
            glm::mat4 topBorder = glm::translate(glm::mat4(1.0f), glm::vec3(barPos.x, barPos.y - borderWidth, 0.0f)) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(barWidth, borderWidth, 1.0f));
            loadingBarMaterial->shader->set("transform", VP * topBorder);
            loadingRectangle->draw();
            
            // Bottom border
            glm::mat4 bottomBorder = glm::translate(glm::mat4(1.0f), glm::vec3(barPos.x, barPos.y + barHeight, 0.0f)) *
                                     glm::scale(glm::mat4(1.0f), glm::vec3(barWidth, borderWidth, 1.0f));
            loadingBarMaterial->shader->set("transform", VP * bottomBorder);
            loadingRectangle->draw();
            
            // Left border
            glm::mat4 leftBorder = glm::translate(glm::mat4(1.0f), glm::vec3(barPos.x - borderWidth, barPos.y - borderWidth, 0.0f)) *
                                   glm::scale(glm::mat4(1.0f), glm::vec3(borderWidth, barHeight + borderWidth * 2, 1.0f));
            loadingBarMaterial->shader->set("transform", VP * leftBorder);
            loadingRectangle->draw();
            
            // Right border
            glm::mat4 rightBorder = glm::translate(glm::mat4(1.0f), glm::vec3(barPos.x + barWidth, barPos.y - borderWidth, 0.0f)) *
                                    glm::scale(glm::mat4(1.0f), glm::vec3(borderWidth, barHeight + borderWidth * 2, 1.0f));
            loadingBarMaterial->shader->set("transform", VP * rightBorder);
            loadingRectangle->draw();
            
            // Progress bar fill (red gradient effect - darker to lighter)
            float fillPadding = 3.0f * scale;
            float fillWidth = (barWidth - fillPadding * 2) * loadingProgress;
            float fillHeight = barHeight - fillPadding * 2;
            
            if (fillWidth > 0) {
                loadingBarMaterial->tint = glm::vec4(0.8f, 0.15f, 0.15f, 1.0f);
                loadingBarMaterial->setup();
                glm::mat4 barFillM = glm::translate(glm::mat4(1.0f), 
                    glm::vec3(barPos.x + fillPadding, barPos.y + fillPadding, 0.0f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(fillWidth, fillHeight, 1.0f));
                loadingBarMaterial->shader->set("transform", VP * barFillM);
                loadingRectangle->draw();
                
                // Lighter highlight on top of progress bar
                loadingBarMaterial->tint = glm::vec4(1.0f, 0.3f, 0.3f, 0.5f);
                loadingBarMaterial->setup();
                glm::mat4 highlightM = glm::translate(glm::mat4(1.0f), 
                    glm::vec3(barPos.x + fillPadding, barPos.y + fillPadding, 0.0f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(fillWidth, fillHeight * 0.4f, 1.0f));
                loadingBarMaterial->shader->set("transform", VP * highlightM);
                loadingRectangle->draw();
            }
            
            // Draw percentage text centered on bar
            int percentage = (int)(loadingProgress * 100.0f);
            std::string percentText = std::to_string(percentage) + "%";
            float percentScale = 0.5f * scale;
            glm::vec2 percentSize = textRenderer->measureText(percentText, percentScale);
            glm::vec2 percentPos = glm::vec2(
                barPos.x + barWidth / 2.0f - percentSize.x / 2.0f, 
                barPos.y + barHeight / 2.0f + percentSize.y / 2.0f 
            );
            textRenderer->renderText(percentText, percentPos, percentScale, 
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), VP);

            // Draw loading stage description
            std::string stageText = "";
            switch (loadingStage) {
                case LoadingStage::LOADING_ASSETS: stageText = "Loading assets..."; break;
                case LoadingStage::LOADING_WORLD: stageText = "Building world..."; break;
                case LoadingStage::INITIALIZING_RENDERER: stageText = "Initializing renderer..."; break;
                case LoadingStage::INITIALIZING_SYSTEMS: stageText = "Starting systems..."; break;
                case LoadingStage::COMPLETE_SPACE: stageText = "Press Space To Continue"; break;
                default: break;
            }
            
            if (!stageText.empty()) {
                float stageScale = 0.4f * scale;
                glm::vec2 stageSize = textRenderer->measureText(stageText, stageScale);
                glm::vec2 stagePos = glm::vec2(
                    barPos.x + barWidth - stageSize.x +5*scale,
                    // Reduce spacing between bar and text below
                    barPos.y + barHeight + 24.0f * scale
                );
                textRenderer->renderText(stageText, stagePos, stageScale, 
                    glm::vec4(0.7f, 0.7f, 0.7f, 0.8f), VP);
            }
        }
    }

    void onDraw(double deltaTime) override {
        // Get keyboard reference at the start
        auto& keyboard = getApp()->getKeyboard();
        
        // Stage 1: Show black screen immediately on first frame
        if (loadingStage == LoadingStage::NOT_STARTED) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            loadingStage = LoadingStage::SHOW_BLACK;
            return;  // Let the frame render
        }
        
        // Stage 2: After black screen is shown, initialize loading UI
        if (loadingStage == LoadingStage::SHOW_BLACK) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            loadingStage = LoadingStage::INIT_LOADING_UI;
            return;  // One more black frame while we prepare
        }
        
        // Stage 3: Initialize loading screen resources
        if (loadingStage == LoadingStage::INIT_LOADING_UI) {
            initializeLoadingResources();
            loadingProgress = 0.0f;
            loadingStage = LoadingStage::LOADING_ASSETS;
            drawLoadingScreen();  // Show initial loading screen
            return;
        }
        
        // Loading stages - draw screen first, then do work
        if (loadingStage == LoadingStage::COMPLETE_SPACE) {
            drawLoadingScreen();
            // Wait for space key to continue
            if (keyboard.justPressed(GLFW_KEY_SPACE)) {
                loadingStage = LoadingStage::COMPLETE;
                cleanupLoadingResources();
            }
            return;
        }
        
        if (loadingStage != LoadingStage::COMPLETE) {
            drawLoadingScreen();
            performLoadingStep();
            return;
        }

        if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            // Lock mouse after
            getApp()->getMouse().lockMouse(getApp()->getWindow());
            // Manually sync mouse position to where lockMouse centered it
            int width, height;
            glfwGetWindowSize(getApp()->getWindow(), &width, &height);
            double centerX = width / 2.0;
            double centerY = height / 2.0;

            // Update the mouse's internal position tracking
            getApp()->getMouse().CursorMoveEvent(centerX, centerY);
            getApp()->getMouse().update();
            paused = !paused;
        }

        if (paused) {
            // Remove navigation logic - just check for ENTER to end game
            if (keyboard.justPressed(GLFW_KEY_ENTER)) {
                paused = false;
                getApp()->changeState("menu"); // End Game
            }

            // Unlock mouse
            getApp()->getMouse().unlockMouse(getApp()->getWindow());

            // Render game state frozen
            renderer.render(&world, 0.0f);

            // Render pause menu
            auto size = getApp()->getFrameBufferSize();
            glm::mat4 projection = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
            
            // Title
            std::string pauseTitle = "PAUSED";
            glm::vec2 titleSize = textRenderer->measureText(pauseTitle, 1.0f);
            glm::vec2 titlePos = glm::vec2(size.x / 2.0f - titleSize.x / 2.0f, size.y / 2.0f - 100);
            textRenderer->renderText(pauseTitle, titlePos, 1.0f, glm::vec4(1.0f), projection);

            // End Game option
            std::string endText = "Press ENTER to End Game";
            glm::vec2 endSize = textRenderer->measureText(endText, 0.7f);
            glm::vec2 endPos = glm::vec2(size.x / 2.0f - endSize.x / 2.0f, size.y / 2.0f);
            textRenderer->renderText(endText, endPos, 0.7f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), projection);

            // ESC hint
            std::string escText = "Press ESC to unpause";
            glm::vec2 escSize = textRenderer->measureText(escText, 0.5f);
            glm::vec2 escPos = glm::vec2(size.x / 2.0f - escSize.x / 2.0f, size.y / 2.0f + 70);
            textRenderer->renderText(escText, escPos, 0.5f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), projection);

            return; 
        }

        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        slendermanAISystem.update(&world, (float)deltaTime, &renderer,
                                  &physicsSystem);
        staticEffectSystem.update(&world, &renderer);
        footstepSystem.update(&world, (float)deltaTime);
        ambientTensionSystem.update(&world, (float)deltaTime);
        staticSoundSystem.update(&world, (float)deltaTime);

        // Update physics
        physicsSystem.update((float)deltaTime);

        // Get camera position and forward for page interaction raycast
        glm::vec3 cameraPos(0), cameraForward(0, 0, -1);
        for (auto entity : world.getEntities()) {
            auto* camera = entity->getComponent<our::CameraComponent>();
            if (camera) {
                glm::mat4 matrix = entity->getLocalToWorldMatrix();
                cameraPos = glm::vec3(matrix[3]);
                cameraForward =
                    glm::normalize(glm::vec3(matrix * glm::vec4(0, 0, -1, 0)));
                break;
            }
        }

        // DEBUG: Print position
        if (keyboard.justPressed(GLFW_KEY_P)) {
            std::cout << "Player Position: (" << cameraPos.x << ", " << cameraPos.y
                      << ", " << cameraPos.z << ")\n";
        }

        // Check for interact key
        bool interactPressed = false;
        // Check if interact key is a mouse button
        if (cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_LEFT ||
            cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_RIGHT ||
            cameraController.getInteractKey() == GLFW_MOUSE_BUTTON_MIDDLE) {
            interactPressed = getApp()->getMouse().justPressed(
                cameraController.getInteractKey());
        } else {
            interactPressed = getApp()->getKeyboard().justPressed(
                cameraController.getInteractKey());
        }
        pageSystem.update(&world, (float)deltaTime, cameraPos, cameraForward,
                          interactPressed);

        textRenderer->updateTimedTexts((float)deltaTime);

        // And finally we use the renderer system to draw the scene
        renderer.render(&world, (float)deltaTime);
        auto size = getApp()->getFrameBufferSize();
        glm::mat4 projection =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f);
        textRenderer->renderTimedTexts(projection);

        // Check if player has collected all pages
        if (pageSystem.allPagesCollected()) {
            getApp()->changeState("win");
        }
        else if (slendermanAISystem.playerIsDead()) {
            getApp()->changeState("death");
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // Destroy physics system
        physicsSystem.destroy();
        // On exit, we call exit for the camera controller system to make sure
        // that the mouse is unlocked
        cameraController.exit();
        // Destroy page system
        pageSystem.destroy();
        if (textRenderer) {
            textRenderer->clearTimedTexts();
            delete textRenderer;
            textRenderer = nullptr;
        }
        // Clean up any remaining loading resources
        cleanupLoadingResources();
        if (loadingRectangle) {
            delete loadingRectangle;
            loadingRectangle = nullptr;
        }
        if (loadingMaterial) {
            delete loadingMaterial->shader;
            delete loadingMaterial;
            loadingMaterial = nullptr;
        }
        if (loadingBarMaterial) {
            delete loadingBarMaterial->shader;
            delete loadingBarMaterial;
            loadingBarMaterial = nullptr;
        }
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the
        // VRAM
        our::clearAllAssets();
    }
};