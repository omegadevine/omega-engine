#include "Sprite.h"
#include "Camera.h"
#include <iostream>
#include <GL/glew.h>

Sprite::Sprite()
    : m_texture(nullptr)
    , m_position(0, 0)
    , m_size(100, 100)
    , m_color(1, 1, 1, 1)
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_buffersInitialized(false) {
}

Sprite::~Sprite() {
    if (m_buffersInitialized) {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }
}

void Sprite::setTexture(Texture* texture) {
    m_texture = texture;
    if (texture && texture->isValid()) {
        // Auto-size to texture dimensions if not manually set
        if (m_size.x == 100 && m_size.y == 100) {
            m_size.x = static_cast<float>(texture->getWidth());
            m_size.y = static_cast<float>(texture->getHeight());
        }
    }
}

void Sprite::setupBuffers() {
    // Vertex data: position (2) + texCoord (2)
    float vertices[] = {
        // positions   // texCoords
        0.0f, 1.0f,    0.0f, 1.0f,  // top-left
        1.0f, 1.0f,    1.0f, 1.0f,  // top-right
        1.0f, 0.0f,    1.0f, 0.0f,  // bottom-right
        0.0f, 0.0f,    0.0f, 0.0f   // bottom-left
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_buffersInitialized = true;
}

void Sprite::draw(Shader* shader, int screenWidth, int screenHeight) {
    if (!shader || !shader->isValid()) {
        return;
    }

    if (!m_buffersInitialized) {
        setupBuffers();
    }

    shader->use();

    // Convert pixel coordinates to normalized device coordinates
    float ndcX = (m_position.x / screenWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (m_position.y / screenHeight) * 2.0f;
    float ndcW = (m_size.x / screenWidth) * 2.0f;
    float ndcH = (m_size.y / screenHeight) * 2.0f;

    // Set uniforms
    GLint posLoc = glGetUniformLocation(shader->getProgramID(), "position");
    GLint sizeLoc = glGetUniformLocation(shader->getProgramID(), "size");
    GLint colorLoc = glGetUniformLocation(shader->getProgramID(), "spriteColor");
    
    if (posLoc != -1) glUniform2f(posLoc, ndcX, ndcY);
    if (sizeLoc != -1) glUniform2f(sizeLoc, ndcW, ndcH);
    if (colorLoc != -1) glUniform4f(colorLoc, m_color.r, m_color.g, m_color.b, m_color.a);

    // Bind texture
    if (m_texture && m_texture->isValid()) {
        m_texture->bind(0);
        GLint texLoc = glGetUniformLocation(shader->getProgramID(), "image");
        if (texLoc != -1) glUniform1i(texLoc, 0);
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);

    shader->unuse();
}

void Sprite::drawWithCamera(Shader* shader, Camera* camera, int screenWidth, int screenHeight) {
    if (!shader || !shader->isValid() || !camera) {
        return;
    }

    if (!m_buffersInitialized) {
        setupBuffers();
    }

    shader->use();

    // Get camera view offset and zoom
    Vector2 viewOffset = camera->getViewOffset();
    float zoom = camera->getViewScale();
    
    // Apply camera transformation to sprite position
    Vector2 cameraPos;
    cameraPos.x = (m_position.x - viewOffset.x) * zoom;
    cameraPos.y = (m_position.y - viewOffset.y) * zoom;
    
    // Apply camera zoom to sprite size
    Vector2 cameraSize;
    cameraSize.x = m_size.x * zoom;
    cameraSize.y = m_size.y * zoom;

    // Convert to NDC
    float ndcX = (cameraPos.x / screenWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (cameraPos.y / screenHeight) * 2.0f;
    float ndcW = (cameraSize.x / screenWidth) * 2.0f;
    float ndcH = (cameraSize.y / screenHeight) * 2.0f;

    // Set uniforms
    GLint posLoc = glGetUniformLocation(shader->getProgramID(), "position");
    GLint sizeLoc = glGetUniformLocation(shader->getProgramID(), "size");
    GLint colorLoc = glGetUniformLocation(shader->getProgramID(), "spriteColor");
    
    if (posLoc != -1) glUniform2f(posLoc, ndcX, ndcY);
    if (sizeLoc != -1) glUniform2f(sizeLoc, ndcW, ndcH);
    if (colorLoc != -1) glUniform4f(colorLoc, m_color.r, m_color.g, m_color.b, m_color.a);

    // Bind texture
    if (m_texture && m_texture->isValid()) {
        m_texture->bind(0);
        GLint texLoc = glGetUniformLocation(shader->getProgramID(), "image");
        if (texLoc != -1) glUniform1i(texLoc, 0);
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);

    shader->unuse();
}
