#include "text-renderer.hpp"
#include <iostream>
 
namespace our {

TextRenderer::TextRenderer() : textShader(nullptr), VAO(0), VBO(0), face(nullptr) {
    // Initialize FreeType
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    
    // Create shader for text rendering
    textShader = new ShaderProgram();
    textShader->attach("assets/shaders/text.vert", GL_VERTEX_SHADER);
    textShader->attach("assets/shaders/text.frag", GL_FRAGMENT_SHADER);
    textShader->link();
    
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Load default font
    loadFont("assets/fonts/arial.ttf", 48);
}

TextRenderer::~TextRenderer() {
    // Clean up character textures
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    
    // Clean up FreeType
    if (face) {
        FT_Done_Face(face);
    }
    FT_Done_FreeType(ft);
    
    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    delete textShader;
}

bool TextRenderer::loadFont(const std::string& fontPath, unsigned int fontSize) {
    // Clear existing characters
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
    
    // Load font face
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
        return false;
    }
    
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph for character: " << c << std::endl;
            continue;
        }
        
        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void TextRenderer::renderText(const std::string& text, glm::vec2 position, float scale, glm::vec4 color, const glm::mat4& projection) {
    if (!textShader) return;
    glBindSampler(0, 0);
    // Activate corresponding render state
    textShader->use();
    textShader->set("projection", projection);
    textShader->set("textColor", color);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Iterate through all characters
    float x = position.x;
    float y = position.y;
    
    for (char c : text) {
        if (characters.find(c) == characters.end()) continue;
        
        Character ch = characters[c];
        
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - ch.bearing.y * scale;
        
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos,     ypos,       0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

glm::vec2 TextRenderer::measureText(const std::string& text, float scale) {
    float width = 0.0f;
    float height = 0.0f;
    
    for (char c : text) {
        if (characters.find(c) == characters.end()) continue;
        Character ch = characters[c];
        width += (ch.advance >> 6) * scale;
        height = std::max(height, ch.size.y * scale);
    }
    
    return glm::vec2(width, height);
}

void TextRenderer::startTimedText(const std::string& text, float duration, glm::vec2 position, float scale, glm::vec4 color) {
    TimedText timedText;
    timedText.text = text;
    timedText.duration = duration;
    timedText.elapsed = 0.0f;
    timedText.position = position;
    timedText.scale = scale;
    timedText.color = color;
    
    timedTexts.push_back(timedText);
}

void TextRenderer::updateTimedTexts(float deltaTime) {
    // Update all timed texts and remove expired ones
    for (auto it = timedTexts.begin(); it != timedTexts.end(); ) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration) {
            it = timedTexts.erase(it);
        } else {
            ++it;
        }
    }
}

void TextRenderer::renderTimedTexts(const glm::mat4& projection) {
    for (auto& timedText : timedTexts) {
        glm::vec4 color = timedText.color;
        
        // Optional: Add fade out effect in the last second
        float fadeTime = 1.0f;
        if (timedText.duration - timedText.elapsed < fadeTime) {
            float alpha = (timedText.duration - timedText.elapsed) / fadeTime;
            color.a *= alpha;
        }
        
        renderText(timedText.text, timedText.position, timedText.scale, color, projection);
    }
}

void TextRenderer::clearTimedTexts() {
    timedTexts.clear();
}

}