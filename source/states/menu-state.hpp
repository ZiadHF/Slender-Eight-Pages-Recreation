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
#include <map>
#include <json/json.hpp>
#include "../common/components/audio-controller.hpp"
#include "../common/systems/text-renderer.hpp"
#include "settings-state.hpp"

// This struct is used to store the location and size of a button and the code
// it should execute when clicked
struct Button {
    // The position (of the top-left corner) of the button and its size in
    // pixels
    glm::vec2 position, size;
    // The function that should be excuted when the button is clicked. It takes
    // no arguments and returns nothing.
    std::function<void()> action;

    // This function returns true if the given vector v is inside the button.
    // Otherwise, false is returned. This is used to check if the mouse is
    // hovering over the button.
    bool isInside(const glm::vec2& v) const {
        return position.x <= v.x && position.y <= v.y &&
               v.x <= position.x + size.x && v.y <= position.y + size.y;
    }

    // This function returns the local to world matrix to transform a rectangle
    // of size 1x1 (and whose top-left corner is at the origin) to be the
    // button.
    glm::mat4 getLocalToWorld() const {
        return glm::translate(glm::mat4(1.0f),
                              glm::vec3(position.x, position.y, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
    }
};

// Struct to hold animated image data
struct AnimatedImage {
    our::Texture2D* texture;
    glm::vec2 startPosition;
    glm::vec2 endPosition;
    glm::vec2 currentPosition;
    float fadeInDuration; // Duration of the fade-in animation
    float moveDuration; // Duration of the move animation
    float fadeOutDuration; // Duration of the fade-out animation
    float currentTime; // Time elapsed since the start of the animation
    float totalDuration; // Duration of the entire animation cycle
    float alpha; // Current alpha value
    float zoom; // Zoom level for the image

    AnimatedImage() : texture(nullptr), currentTime(0), alpha(0), zoom(1.0f) {}
};

// This state shows how to use some of the abstractions we created to make a
// menu.
class Menustate : public our::State {
    // A meterial holding the menu shader and the menu texture to draw
    our::TexturedMaterial* menuMaterial;
    // A material to be used to highlight hovered buttons (we will use blending
    // to create a negative effect).
    our::TintedMaterial* highlightMaterial;
    // A rectangle mesh on which the menu material will be drawn
    our::Mesh* rectangle;
    // A variable to record the time since the state is entered (it will be used
    // for the fading effect).
    float time;
    // An array of the button that we can interact with
    std::array<Button, 3> buttons;

    our::TextRenderer* TextRenderer;
    // New members for animated images
    // We will have a pool of images to choose from, and choose one at random to animate
    std::vector<AnimatedImage> imagePool;
    AnimatedImage* currentImage;
    std::mt19937 rng;
    our::TexturedMaterial* imageMaterial;

    // Audio controller
    our::AudioController* audioController = nullptr;

    // Add helper function to load config
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
        TextRenderer = new our::TextRenderer();
        // First, we create a material for the menu's background
        menuMaterial = new our::TexturedMaterial();
        // Here, we load the shader that will be used to draw the background
        menuMaterial->shader = new our::ShaderProgram();
        menuMaterial->shader->attach("assets/shaders/textured.vert",
                                     GL_VERTEX_SHADER);
        menuMaterial->shader->attach("assets/shaders/textured.frag",
                                     GL_FRAGMENT_SHADER);
        menuMaterial->shader->link();
        // Then we load the menu texture
        menuMaterial->texture =
            our::texture_utils::loadImage("assets/textures/menu.png");
        // Initially, the menu material will be black, then it will fade in
        menuMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        // Second, we create a material to highlight the hovered buttons
        highlightMaterial = new our::TintedMaterial();
        // Since the highlight is not textured, we used the tinted material
        // shaders
        highlightMaterial->shader = new our::ShaderProgram();
        highlightMaterial->shader->attach("assets/shaders/tinted.vert",
                                          GL_VERTEX_SHADER);
        highlightMaterial->shader->attach("assets/shaders/tinted.frag",
                                          GL_FRAGMENT_SHADER);
        highlightMaterial->shader->link();
        // The tint is white since we will subtract the background color from it
        // to create a negative effect.
        highlightMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // To create a negative effect, we enable blending, set the equation to
        // be subtract, and set the factors to be one for both the source and
        // the destination.
        highlightMaterial->pipelineState.blending.enabled = true;
        highlightMaterial->pipelineState.blending.equation = GL_FUNC_SUBTRACT;
        highlightMaterial->pipelineState.blending.sourceFactor = GL_ONE;
        highlightMaterial->pipelineState.blending.destinationFactor = GL_ONE;

        // Clear any existing images in the pool
        for (auto& img : imagePool) {
            if (img.texture) {
                delete img.texture;
            }
        }
        imagePool.clear();
        currentImage = nullptr;

        // Check if audio controller is already initialized and clear it
        if (audioController) {
            delete audioController;
            audioController = nullptr;
        }

        // Initialize the audio controller
        audioController = new our::AudioController();
        if (audioController->initializeMusic("assets/sounds/menu.wav", true)) {
            audioController->setVolume(0.5f);
            audioController->playMusic();
        }

        // Then we create a rectangle whose top-left corner is at the origin and
        // its size is 1x1. Note that the texture coordinates at the origin is
        // (0.0, 1.0) since we will use the projection matrix to make the origin
        // at the the top-left corner of the screen.
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

        // Reset the time elapsed since the state is entered.
        time = 0;

        // Fill the positions, sizes and actions for the menu buttons
        // Note that we use lambda expressions to set the actions of the
        // buttons. A lambda expression consists of 3 parts:
        // - The capture list [] which is the variables that the lambda should
        // remember because it will use them during execution.
        //      We store [this] in the capture list since we will use it in the
        //      action.
        // - The argument list () which is the arguments that the lambda should
        // receive when it is called.
        //      We leave it empty since button actions receive no input.
        // - The body {} which contains the code to be executed.
        
        // Adjustments for different window sizes
        float scaleX = getApp()->getFrameBufferSize().x / 1280.0f;
        float scaleY = getApp()->getFrameBufferSize().y / 720.0f;
        float scaleSize = (scaleX + scaleY) / 2.0f;

        buttons[0].position = {441.0f * scaleX, 573.0f * scaleY}; // This is for a 1280x720 window so we have to adjust it if the window size is different
        buttons[0].size = {400.0f * scaleSize, 33.0f * scaleSize}; // Size has to be adjusted too so it can take the correct relative space
        buttons[0].action = [this]() { this->getApp()->changeState("play");};

        buttons[1].position = {441.0f * scaleX, 608.0f * scaleY};
        buttons[1].size = {400.0f * scaleSize, 33.0f * scaleSize};
        buttons[1].action = [this]() { this->getApp()->changeState("settings"); };

        buttons[2].position = {441.0f * scaleX, 643.0f * scaleY};
        buttons[2].size = {400.0f * scaleSize, 33.0f * scaleSize};
        buttons[2].action = [this]() { this->getApp()->close(); };

        // Initialize random number generator
        rng.seed(std::random_device{}());

        // Create material for animated images
        imageMaterial = new our::TexturedMaterial();
        imageMaterial->shader = new our::ShaderProgram();
        imageMaterial->shader->attach("assets/shaders/textured.vert",
                                      GL_VERTEX_SHADER);
        imageMaterial->shader->attach("assets/shaders/textured.frag",
                                      GL_FRAGMENT_SHADER);
        imageMaterial->shader->link();
        imageMaterial->pipelineState.blending.enabled = true;
        imageMaterial->pipelineState.blending.equation = GL_FUNC_ADD;
        imageMaterial->pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        imageMaterial->pipelineState.blending.destinationFactor =
            GL_ONE_MINUS_SRC_ALPHA;

        // Load multiple images for the animation pool
        // Add your image paths here
        std::vector<std::string> imagePaths = {
            "assets/textures/menu/sketch_1.png",
            "assets/textures/menu/sketch_2.png",
            "assets/textures/menu/sketch_3.png",
            "assets/textures/menu/sketch_4.png",
            "assets/textures/menu/sketch_5.png",
            "assets/textures/menu/sketch_6.png",
            "assets/textures/menu/sketch_7.png",
            "assets/textures/menu/sketch_8.png"};

        for (const auto& path : imagePaths) {
            AnimatedImage img;
            img.texture = our::texture_utils::loadImage(path);
            img.fadeInDuration = 0.5f;
            img.moveDuration = 3.0f;
            img.fadeOutDuration = 0.5f;
            img.totalDuration =
                img.fadeInDuration + img.moveDuration + img.fadeOutDuration;
            imagePool.push_back(img);
        }

        currentImage = nullptr;
        startNewImageAnimation();
    }

    // Function to start a new animated image
    void startNewImageAnimation() {
        if (imagePool.empty()) return;

        // Pick random image
        std::uniform_int_distribution<> dist(0, imagePool.size() - 1);
        int index = dist(rng);
        currentImage = &imagePool[index];
        currentImage->currentTime = 0;

        // Get window size for positioning
        glm::ivec2 size = getApp()->getFrameBufferSize();

        // Random zoom between 1.8x and 2.5x as I found these values visually appealing
        std::uniform_real_distribution<float> zoomDist(1.8f, 2.5f);
        float zoom = zoomDist(rng);
        currentImage->zoom = zoom;  // Store the zoom
        float imageWidth = 400.0f * zoom;
        float imageHeight = 600.0f * zoom;

        // Position away from center - only corners and far edges to avoid clutter
        std::uniform_int_distribution<> zoneDist(0, 7);
        int zone = zoneDist(rng);

        std::uniform_real_distribution<float> edgeVariation(0.0f, 0.15f);
        float varX = edgeVariation(rng);
        float varY = edgeVariation(rng);

        switch (zone) {
            case 0:  // Far top-left corner
                currentImage->startPosition =
                    glm::vec2(size.x * (0.0f + varX * 0.1f),
                              size.y * (0.0f + varY * 0.1f));
                break;
            case 1:  // Far top-right corner
                currentImage->startPosition =
                    glm::vec2(size.x * (0.9f - varX * 0.1f) - imageWidth,
                              size.y * (0.0f + varY * 0.1f));
                break;
            case 2:  // Far bottom-left corner
                currentImage->startPosition =
                    glm::vec2(size.x * (0.0f + varX * 0.1f),
                              size.y * (0.9f - varY * 0.1f) - imageHeight);
                break;
            case 3:  // Far bottom-right corner
                currentImage->startPosition =
                    glm::vec2(size.x * (0.9f - varX * 0.1f) - imageWidth,
                              size.y * (0.9f - varY * 0.1f) - imageHeight);
                break;
            case 4:  // Far left edge
                currentImage->startPosition = glm::vec2(
                    size.x * (0.0f + varX * 0.05f),
                    size.y * (varY < 0.075f ? 0.1f : 0.8f) * (1.0f - varY));
                break;
            case 5:  // Far right edge
                currentImage->startPosition = glm::vec2(
                    size.x * (0.95f - varX * 0.05f) - imageWidth,
                    size.y * (varY < 0.075f ? 0.1f : 0.8f) * (1.0f - varY));
                break;
            case 6:  // Top edge
                currentImage->startPosition = glm::vec2(
                    size.x * (varX < 0.075f ? 0.05f : 0.85f) - imageWidth / 2,
                    size.y * (0.0f + varY * 0.05f));
                break;
            case 7:  // Bottom edge
                currentImage->startPosition = glm::vec2(
                    size.x * (varX < 0.075f ? 0.05f : 0.85f) - imageWidth / 2,
                    size.y * (0.95f - varY * 0.05f) - imageHeight);
                break;
        }

        currentImage->endPosition = currentImage->startPosition;

        // Add small drift movement
        std::uniform_real_distribution<float> driftDist(-30.0f, 30.0f);
        currentImage->endPosition.x += driftDist(rng);
        currentImage->endPosition.y += driftDist(rng);

        currentImage->currentPosition = currentImage->startPosition;

        // Store the zoom for rendering
        currentImage->fadeInDuration = 1.5f;
        currentImage->moveDuration = 6.0f;
        currentImage->fadeOutDuration = 1.5f;
        currentImage->totalDuration = currentImage->fadeInDuration +
        currentImage->moveDuration +
        currentImage->fadeOutDuration;
    }
    
    void onDraw(double deltaTime) override {
        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();
        // Get the framebuffer size to set the viewport and the create the
        // projection matrix.
        glm::ivec2 size = getApp()->getFrameBufferSize();
        // Make sure the viewport covers the whole size of the framebuffer.
        glViewport(0, 0, size.x, size.y);
    
        // The view matrix is an identity (there is no camera that moves
        // around). The projection matrix applys an orthographic projection
        // whose size is the framebuffer size in pixels so that the we can
        // define our object locations and sizes in pixels. Note that the top is
        // at 0.0 and the bottom is at the framebuffer height. This allows us to
        // consider the top-left corner of the window to be the origin which
        // makes dealing with the mouse input easier.
        glm::mat4 VP =
            glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        // The local to world (model) matrix of the background which is just a
        // scaling matrix to make the menu cover the whole window. Note that we
        // defind the scale in pixels.
        glm::mat4 M =
            glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        if (keyboard.justPressed(GLFW_KEY_SPACE)) {
            getApp()->changeState("play");
            return;
            
        } else if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            // If the escape key is pressed in this frame, exit the game
            getApp()->close();
        }
        else if (keyboard.justPressed(GLFW_KEY_S)) {
            // If the S key is pressed in this frame, go to the settings state
            getApp()->changeState("settings");
        }

        // Get a reference to the mouse object and get the current mouse
        // position
        auto& mouse = getApp()->getMouse();
        glm::vec2 mousePosition = mouse.getMousePosition();

        // If the mouse left-button is just pressed, check if the mouse was
        // inside any menu button. If it was inside a menu button, run the
        // action of the button.
        if (mouse.justPressed(0)){
            for (size_t i = 0; i < buttons.size(); i++)
            {
                if (buttons[i].isInside(mousePosition)){
                    buttons[i].action();
                    if (i == 0)
                    { 
                        return;
                    }
                }
            }
        }

        // First, we apply the fading effect.
        time += (float)deltaTime;
        menuMaterial->tint = glm::vec4(glm::smoothstep(0.00f, 2.00f, time));
        // Then we render the menu background
        // Notice that I don't clear the screen first, since I assume that the
        // menu rectangle will draw over the whole window anyway.
        menuMaterial->setup();
        menuMaterial->shader->set("transform", VP * M);
        rectangle->draw();

        // Update and draw animated image
        if (currentImage) {
            currentImage->currentTime += (float)deltaTime;

            float t = currentImage->currentTime;
            float maxAlpha = 0.1f;  // Base transparency (10% opacity to make it more subtle and avoid clashing with menu)

            // Calculate alpha based on phase
            if (t < currentImage->fadeInDuration) {
                currentImage->alpha =
                    (t / currentImage->fadeInDuration) * maxAlpha;
            } else if (t < currentImage->fadeInDuration +
                               currentImage->moveDuration) {
                currentImage->alpha = maxAlpha;
            } else if (t < currentImage->totalDuration) {
                float fadeT = (t - currentImage->fadeInDuration -
                               currentImage->moveDuration) /
                              currentImage->fadeOutDuration;
                currentImage->alpha = (1.0f - fadeT) * maxAlpha;
            } else {
                // Animation finished, start new one
                startNewImageAnimation();
                return;
            }

            // Calculate position with slow drift
            float moveT = glm::clamp(
                (t - currentImage->fadeInDuration) / currentImage->moveDuration,
                0.0f, 1.0f);
            currentImage->currentPosition = glm::mix(
                currentImage->startPosition, currentImage->endPosition, moveT);

            // Draw the image (zoomed in)
            imageMaterial->texture = currentImage->texture;
            imageMaterial->tint =
                glm::vec4(1.0f, 1.0f, 1.0f, currentImage->alpha);
            imageMaterial->setup();

            glm::mat4 imageM =
                glm::translate(glm::mat4(1.0f),
                               glm::vec3(currentImage->currentPosition, 0.0f)) *
                glm::scale(glm::mat4(1.0f),
                           glm::vec3(400.0f * currentImage->zoom,
                                     600.0f * currentImage->zoom,
                                     1.0f));  // Increased scale
            imageMaterial->shader->set("transform", VP * imageM);
            rectangle->draw();
        }

        // For every button, check if the mouse is inside it. If the mouse is
        // inside, we draw the highlight rectangle over it.
        for (auto& button : buttons) {
            if (button.isInside(mousePosition)) {
                highlightMaterial->setup();
                highlightMaterial->shader->set("transform",
                                               VP * button.getLocalToWorld());
                rectangle->draw();
            }
        }
    }

    void onDestroy() override {
        // Delete all the allocated resources
        delete rectangle;
        delete menuMaterial->texture;
        delete menuMaterial->shader;
        delete menuMaterial;
        delete highlightMaterial->shader;
        delete highlightMaterial;

        // Clean up audio resources
        if (audioController) {
            audioController->stopMusic();
            delete audioController;
            audioController = nullptr;
        }

        // Clean up animated images
        for (auto& img : imagePool) {
            delete img.texture;
            img.texture = nullptr;
        }
        delete imageMaterial->shader;
        delete imageMaterial;
    }
};