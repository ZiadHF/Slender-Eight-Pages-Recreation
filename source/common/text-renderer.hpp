#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include "texture/texture2d.hpp"
#include "shader/shader.hpp"
#include "mesh/mesh.hpp"
#include "material/material.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace our {

struct Character {
    unsigned int textureID;    // ID handle of the glyph texture
    glm::ivec2 size;          // Size of glyph
    glm::ivec2 bearing;       // Offset from baseline to left/top of glyph
    unsigned int advance;     // Horizontal offset to advance to next glyph
};

class TextRenderer {
private:
    ShaderProgram* textShader;
    unsigned int VAO, VBO;
    std::map<char, Character> characters;
    FT_Library ft;
    FT_Face face;
    
public:
    TextRenderer();
    ~TextRenderer();
    
    bool loadFont(const std::string& fontPath, unsigned int fontSize = 48);
    void renderText(const std::string& text, glm::vec2 position, float scale, glm::vec4 color, const glm::mat4& projection);
    glm::vec2 measureText(const std::string& text, float scale);
};

}
