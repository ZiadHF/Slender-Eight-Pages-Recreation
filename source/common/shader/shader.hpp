#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

namespace our {

    class ShaderProgram {

    private:
        //Shader Program Handle (OpenGL object name)
        GLuint program;

    public:
        ShaderProgram(){
            program = glCreateProgram();
        }
        ~ShaderProgram(){
            glDeleteProgram(program);
        }

        bool attach(const std::string &filename, GLenum type) const;

        bool link() const;

        void use() { 
            glUseProgram(program);
        }

        GLuint getUniformLocation(const std::string &name) {
            return glGetUniformLocation(program, name.c_str());
        }

        void set(const std::string &uniform, GLfloat value) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform1f(loc, value);
            }
        }

        void set(const std::string &uniform, GLuint value) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform1ui(loc, value);
            }
        }

        void set(const std::string &uniform, GLint value) {
            GLint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform1i(loc, value);
            }
        }

        void set(const std::string &uniform, glm::vec2 value) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform2f(loc, value.x, value.y);
            }
        }

        void set(const std::string &uniform, glm::vec3 value) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform3f(loc, value.x, value.y, value.z);
            }
        }

        void set(const std::string &uniform, glm::vec4 value) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniform4f(loc, value.x, value.y, value.z, value.w);
            }
        }

        void set(const std::string &uniform, glm::mat4 matrix) {
            GLuint loc = getUniformLocation(uniform);
            if (loc != GL_INVALID_INDEX) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
            }
        }

        // Disable copy constructor and assignment operator
        ShaderProgram(const ShaderProgram&) = delete; 
        ShaderProgram& operator=(const ShaderProgram&) = delete;
    };

}

#endif