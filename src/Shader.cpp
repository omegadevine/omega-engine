#include "Shader.h"
#include <iostream>
#include <vector>
#include <GL/glew.h>

Shader::Shader()
    : m_programID(0) {
}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, errorLog.data());
        std::cerr << "Shader compilation failed: " << errorLog.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    // Check linking status
    GLint success = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetProgramInfoLog(m_programID, logLength, &logLength, errorLog.data());
        std::cerr << "Shader linking failed: " << errorLog.data() << std::endl;
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    return true;
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) {
        return false;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    bool linkSuccess = linkProgram(vertexShader, fragmentShader);

    // Clean up shaders (they're now in the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (linkSuccess) {
        std::cout << "Shader program created successfully (ID: " << m_programID << ")" << std::endl;
    }

    return linkSuccess;
}

void Shader::use() {
    if (m_programID != 0) {
        glUseProgram(m_programID);
    }
}

void Shader::unuse() {
    glUseProgram(0);
}
