#include "AnimatedSprite.h"
#include "Camera.h"
#include <iostream>

AnimatedSprite::AnimatedSprite()
    : m_currentAnimationName("")
    , m_currentAnimation(nullptr)
    , m_currentFrame(0)
    , m_frameTimer(0.0f)
    , m_playing(false)
    , m_paused(false)
    , m_texture(nullptr)
    , m_baseSize(64, 64) {
}

void AnimatedSprite::addAnimation(const std::string& name, const Animation& animation) {
    m_animations[name] = animation;
}

void AnimatedSprite::play(const std::string& name, bool restart) {
    auto it = m_animations.find(name);
    if (it == m_animations.end()) {
        std::cerr << "Animation '" << name << "' not found" << std::endl;
        return;
    }
    
    // If already playing this animation and not restarting, do nothing
    if (m_currentAnimationName == name && m_playing && !restart) {
        return;
    }
    
    m_currentAnimationName = name;
    m_currentAnimation = &it->second;
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
    m_playing = true;
    m_paused = false;
    
    updateSpriteFrame();
}

void AnimatedSprite::pause() {
    m_paused = true;
}

void AnimatedSprite::resume() {
    m_paused = false;
}

void AnimatedSprite::stop() {
    m_playing = false;
    m_paused = false;
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
}

void AnimatedSprite::update(float deltaTime) {
    if (!m_playing || m_paused || !m_currentAnimation) {
        return;
    }
    
    m_frameTimer += deltaTime;
    
    if (m_frameTimer >= m_currentAnimation->getFrameDuration()) {
        m_frameTimer = 0.0f;
        m_currentFrame++;
        
        // Check if animation finished
        if (m_currentFrame >= static_cast<int>(m_currentAnimation->getFrameCount())) {
            if (m_currentAnimation->isLooping()) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = static_cast<int>(m_currentAnimation->getFrameCount()) - 1;
                m_playing = false;
                m_currentAnimation->triggerComplete();
            }
        }
        
        updateSpriteFrame();
    }
}

void AnimatedSprite::setTexture(Texture* texture) {
    m_texture = texture;
    m_sprite.setTexture(texture);
}

void AnimatedSprite::setPosition(const Vector2& pos) {
    m_sprite.setPosition(pos);
}

void AnimatedSprite::setSize(const Vector2& size) {
    m_baseSize = size;
    m_sprite.setSize(size);
}

void AnimatedSprite::setColor(const Color& color) {
    m_sprite.setColor(color);
}

Vector2 AnimatedSprite::getPosition() const {
    return m_sprite.getPosition();
}

Vector2 AnimatedSprite::getSize() const {
    return m_sprite.getSize();
}

void AnimatedSprite::draw(Shader* shader, int screenWidth, int screenHeight) {
    m_sprite.draw(shader, screenWidth, screenHeight);
}

void AnimatedSprite::drawWithCamera(Shader* shader, Camera* camera, int screenWidth, int screenHeight) {
    m_sprite.drawWithCamera(shader, camera, screenWidth, screenHeight);
}

void AnimatedSprite::updateSpriteFrame() {
    if (!m_currentAnimation || !m_texture) {
        return;
    }
    
    // Get current frame data
    AnimFrame frame = m_currentAnimation->getFrame(m_currentFrame);
    
    // For now, we'll just store the frame data
    // In a full implementation, you'd update texture coordinates here
    // This is a simplified version that works with full sprite sheets
    
    // You could implement texture coordinate updates in Sprite class
    // or use a shader-based approach with UV offsets
}
