#ifndef OMEGA_CAMERA_H
#define OMEGA_CAMERA_H

#include "Sprite.h"
#include <cmath>

class Camera {
public:
    Camera(float screenWidth, float screenHeight);
    ~Camera() = default;

    // Update camera (call once per frame)
    void update(float deltaTime);

    // Position control
    void setPosition(const Vector2& pos);
    Vector2 getPosition() const { return m_position; }
    
    // Follow target
    void follow(const Vector2& targetPos);
    void setFollowSpeed(float speed) { m_followSpeed = speed; }
    float getFollowSpeed() const { return m_followSpeed; }
    
    // Zoom control
    void setZoom(float zoom);
    float getZoom() const { return m_zoom; }
    void zoomIn(float amount);
    void zoomOut(float amount);
    
    // Bounds (optional - constrains camera movement)
    void setBounds(float minX, float minY, float maxX, float maxY);
    void clearBounds();
    bool hasBounds() const { return m_hasBounds; }
    
    // Screen shake effect
    void shake(float intensity, float duration);
    
    // Coordinate transformations
    Vector2 screenToWorld(const Vector2& screenPos) const;
    Vector2 worldToScreen(const Vector2& worldPos) const;
    
    // Get view matrix components (for rendering)
    Vector2 getViewOffset() const;
    float getViewScale() const { return m_zoom; }
    
    // Reset camera
    void reset();

private:
    void applyBounds();
    
    Vector2 m_position;
    Vector2 m_targetPosition;
    float m_zoom;
    float m_followSpeed;
    
    float m_screenWidth;
    float m_screenHeight;
    
    // Bounds
    bool m_hasBounds;
    float m_minX, m_minY, m_maxX, m_maxY;
    
    // Screen shake
    float m_shakeIntensity;
    float m_shakeDuration;
    float m_shakeTimer;
    Vector2 m_shakeOffset;
};

#endif // OMEGA_CAMERA_H
