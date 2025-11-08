#ifndef OMEGA_SHADER_H
#define OMEGA_SHADER_H

#include <string>
#include <SDL_opengl.h>

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void use();
    void unuse();
    
    GLuint getProgramID() const { return m_programID; }
    bool isValid() const { return m_programID != 0; }

private:
    GLuint compileShader(GLenum type, const std::string& source);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    GLuint m_programID;
};

#endif // OMEGA_SHADER_H
