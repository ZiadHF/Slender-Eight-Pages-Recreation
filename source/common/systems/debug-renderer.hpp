#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include "../shader/shader.hpp"
#include <vector>
#include <iostream>

namespace our {

class DebugRenderer {
private:
    ShaderProgram* debugShader = nullptr;
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    GLuint sphereEBO = 0;
    GLuint crosshairVAO = 0;
    GLuint crosshairVBO = 0;
    int sphereIndexCount = 0;

public:
    void initialize() {
        // Create debug shader
        debugShader = new ShaderProgram();
        debugShader->attach("assets/shaders/debug.vert", GL_VERTEX_SHADER);
        debugShader->attach("assets/shaders/debug.frag", GL_FRAGMENT_SHADER);
        debugShader->link();

        // Create sphere mesh
        createSphereMesh(16, 16);
        
        // Create crosshair VAO
        createCrosshairMesh();
    }

    void createSphereMesh(int stacks, int slices) {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= stacks; ++i) {
            float phi = glm::pi<float>() * float(i) / float(stacks);
            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * glm::pi<float>() * float(j) / float(slices);
                
                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);
                
                vertices.push_back(glm::vec3(x, y, z));
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                int first = i * (slices + 1) + j;
                int second = first + slices + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        sphereIndexCount = indices.size();

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(sphereVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }

    void createCrosshairMesh() {
        glGenVertexArrays(1, &crosshairVAO);
        glGenBuffers(1, &crosshairVBO);
    }

    void drawSphere(const glm::vec3& position, float radius, const glm::vec4& color, const glm::mat4& VP) {
        if (!debugShader) return;

        debugShader->use();
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::scale(model, glm::vec3(radius));
        
        debugShader->set("MVP", VP * model);
        debugShader->set("color", color);

        glBindVertexArray(sphereVAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);
    }

    // Draw a 2D crosshair at screen center
    // canInteract: true = green (can collect), false = white (normal)
    void drawCrosshair(const glm::ivec2& screenSize, bool canInteract, float distance = -1.0f) {
        if (!debugShader) return;

        float centerX = screenSize.x / 2.0f;
        float centerY = screenSize.y / 2.0f;
        
        // Crosshair size
        float size = canInteract ? 15.0f : 10.0f;  // Larger when can interact
        float thickness = 2.0f;
        
        // Choose color based on state
        glm::vec4 color;
        if (canInteract) {
            color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green - can collect
        } else if (distance >= 0.0f && distance < 3.0f) {
            // Yellow-ish when close but not quite in range
            float t = distance / 3.0f;
            color = glm::vec4(1.0f, 1.0f - t * 0.5f, 0.0f, 1.0f);
        } else {
            color = glm::vec4(1.0f, 1.0f, 1.0f, 0.7f);  // White with some transparency
        }

        // Create crosshair vertices (two rectangles forming a +)
        std::vector<glm::vec3> vertices = {
            // Horizontal bar
            {centerX - size, centerY - thickness/2, 0},
            {centerX + size, centerY - thickness/2, 0},
            {centerX + size, centerY + thickness/2, 0},
            {centerX - size, centerY + thickness/2, 0},
            // Vertical bar
            {centerX - thickness/2, centerY - size, 0},
            {centerX + thickness/2, centerY - size, 0},
            {centerX + thickness/2, centerY + size, 0},
            {centerX - thickness/2, centerY + size, 0},
        };

        // If can interact, add corner brackets for emphasis
        if (canInteract) {
            float bracketSize = 20.0f;
            float bracketThickness = 3.0f;
            float offset = 25.0f;
            
            // Top-left bracket
            vertices.push_back({centerX - offset - bracketSize, centerY - offset, 0});
            vertices.push_back({centerX - offset, centerY - offset, 0});
            vertices.push_back({centerX - offset, centerY - offset + bracketThickness, 0});
            vertices.push_back({centerX - offset - bracketSize, centerY - offset + bracketThickness, 0});
            
            vertices.push_back({centerX - offset, centerY - offset - bracketSize, 0});
            vertices.push_back({centerX - offset + bracketThickness, centerY - offset - bracketSize, 0});
            vertices.push_back({centerX - offset + bracketThickness, centerY - offset, 0});
            vertices.push_back({centerX - offset, centerY - offset, 0});
            
            // Top-right bracket
            vertices.push_back({centerX + offset, centerY - offset, 0});
            vertices.push_back({centerX + offset + bracketSize, centerY - offset, 0});
            vertices.push_back({centerX + offset + bracketSize, centerY - offset + bracketThickness, 0});
            vertices.push_back({centerX + offset, centerY - offset + bracketThickness, 0});
            
            vertices.push_back({centerX + offset - bracketThickness, centerY - offset - bracketSize, 0});
            vertices.push_back({centerX + offset, centerY - offset - bracketSize, 0});
            vertices.push_back({centerX + offset, centerY - offset, 0});
            vertices.push_back({centerX + offset - bracketThickness, centerY - offset, 0});
            
            // Bottom-left bracket
            vertices.push_back({centerX - offset - bracketSize, centerY + offset - bracketThickness, 0});
            vertices.push_back({centerX - offset, centerY + offset - bracketThickness, 0});
            vertices.push_back({centerX - offset, centerY + offset, 0});
            vertices.push_back({centerX - offset - bracketSize, centerY + offset, 0});
            
            vertices.push_back({centerX - offset, centerY + offset, 0});
            vertices.push_back({centerX - offset + bracketThickness, centerY + offset, 0});
            vertices.push_back({centerX - offset + bracketThickness, centerY + offset + bracketSize, 0});
            vertices.push_back({centerX - offset, centerY + offset + bracketSize, 0});
            
            // Bottom-right bracket
            vertices.push_back({centerX + offset, centerY + offset - bracketThickness, 0});
            vertices.push_back({centerX + offset + bracketSize, centerY + offset - bracketThickness, 0});
            vertices.push_back({centerX + offset + bracketSize, centerY + offset, 0});
            vertices.push_back({centerX + offset, centerY + offset, 0});
            
            vertices.push_back({centerX + offset - bracketThickness, centerY + offset, 0});
            vertices.push_back({centerX + offset, centerY + offset, 0});
            vertices.push_back({centerX + offset, centerY + offset + bracketSize, 0});
            vertices.push_back({centerX + offset - bracketThickness, centerY + offset + bracketSize, 0});
        }

        // Orthographic projection for 2D
        glm::mat4 ortho = glm::ortho(0.0f, (float)screenSize.x, (float)screenSize.y, 0.0f);

        debugShader->use();
        debugShader->set("MVP", ortho);
        debugShader->set("color", color);

        glBindVertexArray(crosshairVAO);
        glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // Draw as quads (2 triangles per quad)
        int numQuads = vertices.size() / 4;
        for (int i = 0; i < numQuads; i++) {
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        }
        
        glBindVertexArray(0);
    }

    void destroy() {
        if (debugShader) {
            delete debugShader;
            debugShader = nullptr;
        }
        if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
        if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
        if (sphereEBO) glDeleteBuffers(1, &sphereEBO);
        if (crosshairVAO) glDeleteVertexArrays(1, &crosshairVAO);
        if (crosshairVBO) glDeleteBuffers(1, &crosshairVBO);
    }

    ~DebugRenderer() {
        destroy();
    }
};

} // namespace our
