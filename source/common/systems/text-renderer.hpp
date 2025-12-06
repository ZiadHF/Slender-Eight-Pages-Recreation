#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader/shader.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include <vector>

namespace our {

struct Character {
    unsigned int textureID;  // ID handle of the glyph texture
    glm::ivec2 size;        // Size of glyph
    glm::ivec2 bearing;     // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

struct TimedText {
    std::string text;
    float duration;
    float elapsed;
    glm::vec2 position;
    float scale;
    glm::vec4 color;
};

class TextRenderer {
private:
    FT_Library ft;
    FT_Face face;
    ShaderProgram* textShader;
    unsigned int VAO, VBO;
    std::map<char, Character> characters;
    std::vector<TimedText> timedTexts;

public:
    TextRenderer();
    ~TextRenderer();
    
    bool loadFont(const std::string& fontPath, unsigned int fontSize);
    void renderText(const std::string& text, glm::vec2 position, float scale, glm::vec4 color, const glm::mat4& projection);
    glm::vec2 measureText(const std::string& text, float scale);
    
    // Timed text methods
    void startTimedText(const std::string& text, float duration, glm::vec2 position, float scale, glm::vec4 color);
    void updateTimedTexts(float deltaTime);
    void renderTimedTexts(const glm::mat4& projection);
    void clearTimedTexts();
};

}