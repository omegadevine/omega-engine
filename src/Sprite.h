#ifndef OMEGA_SPRITE_H
#define OMEGA_SPRITE_H

#include "Texture.h"
#include "Shader.h"
#include <SDL_opengl.h>

struct Vector2 {
    float x, y;
    Vector2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

struct Color {
    float r, g, b, a;
    Color(float _r = 1.0f, float _g = 1.0f, float _b = 1.0f, float _a = 1.0f) 
        : r(_r), g(_g), b(_b), a(_a) {}
};

class Sprite {
public:
    Sprite();
    ~Sprite();

    void setTexture(Texture* texture);
    void setPosition(const Vector2& pos) { m_position = pos; }
    void setSize(const Vector2& size) { m_size = size; }
    void setColor(const Color& color) { m_color = color; }
    
    Vector2 getPosition() const { return m_position; }
    Vector2 getSize() const { return m_size; }
    
    void draw(Shader* shader, int screenWidth, int screenHeight);

private:
    void setupBuffers();
    
    Texture* m_texture;
    Vector2 m_position;
    Vector2 m_size;
    Color m_color;
    
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    bool m_buffersInitialized;
};

#endif // OMEGA_SPRITE_H
