#include "Texture.h"
#include <iostream>
#include <GL/glew.h>

// For now, we'll use a simple stub implementation
// In production, you'd use stb_image or SDL_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
    : m_textureID(0)
    , m_width(0)
    , m_height(0) {
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

bool Texture::loadFromFile(const std::string& filepath) {
    int channels;
    unsigned char* data = stbi_load(filepath.c_str(), &m_width, &m_height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    bool success = createFromData(data, m_width, m_height, channels);
    stbi_image_free(data);
    
    if (success) {
        std::cout << "Loaded texture: " << filepath << " (" << m_width << "x" << m_height << ", " << channels << " channels)" << std::endl;
    }
    
    return success;
}

bool Texture::createFromData(unsigned char* data, int width, int height, int channels) {
    if (!data) {
        std::cerr << "Cannot create texture from null data" << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine format
    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Texture::bind(unsigned int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
