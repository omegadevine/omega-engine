#ifndef OMEGA_ANIMATED_SPRITE_H
#define OMEGA_ANIMATED_SPRITE_H

#include "Sprite.h"
#include "Animation.h"
#include <map>
#include <string>

class AnimatedSprite {
public:
    AnimatedSprite();
    ~AnimatedSprite() = default;

    // Animation management
    void addAnimation(const std::string& name, const Animation& animation);
    void play(const std::string& name, bool restart = false);
    void pause();
    void resume();
    void stop();
    
    std::string getCurrentAnimation() const { return m_currentAnimationName; }
    bool isPlaying() const { return m_playing; }
    bool isPaused() const { return m_paused; }
    
    // Update animation (call each frame)
    void update(float deltaTime);
    
    // Sprite properties
    void setTexture(Texture* texture);
    void setPosition(const Vector2& pos);
    void setSize(const Vector2& size);
    void setColor(const Color& color);
    
    Vector2 getPosition() const;
    Vector2 getSize() const;
    
    // Rendering
    void draw(Shader* shader, int screenWidth, int screenHeight);
    void drawWithCamera(Shader* shader, Camera* camera, int screenWidth, int screenHeight);
    
    // Get underlying sprite
    Sprite& getSprite() { return m_sprite; }

private:
    void updateSpriteFrame();
    
    Sprite m_sprite;
    std::map<std::string, Animation> m_animations;
    
    std::string m_currentAnimationName;
    Animation* m_currentAnimation;
    
    int m_currentFrame;
    float m_frameTimer;
    bool m_playing;
    bool m_paused;
    
    Texture* m_texture;
    Vector2 m_baseSize; // Original sprite size before animation frames
};

#endif // OMEGA_ANIMATED_SPRITE_H
