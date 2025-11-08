#ifndef OMEGA_TEXTURE_H
#define OMEGA_TEXTURE_H

#include <SDL_opengl.h>
#include <string>

class Texture {
public:
    Texture();
    ~Texture();

    bool loadFromFile(const std::string& filepath);
    bool createFromData(unsigned char* data, int width, int height, int channels);
    void bind(unsigned int slot = 0);
    void unbind();
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLuint getID() const { return m_textureID; }
    bool isValid() const { return m_textureID != 0; }

private:
    GLuint m_textureID;
    int m_width;
    int m_height;
};

#endif // OMEGA_TEXTURE_H
