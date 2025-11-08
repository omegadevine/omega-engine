#include "Camera.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

Camera::Camera(float screenWidth, float screenHeight)
    : m_position(screenWidth / 2.0f, screenHeight / 2.0f)
    , m_targetPosition(m_position)
    , m_zoom(1.0f)
    , m_followSpeed(5.0f)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_hasBounds(false)
    , m_minX(0), m_minY(0), m_maxX(0), m_maxY(0)
    , m_shakeIntensity(0.0f)
    , m_shakeDuration(0.0f)
    , m_shakeTimer(0.0f)
    , m_shakeOffset(0, 0) {
    
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void Camera::update(float deltaTime) {
    // Smooth follow to target position
    if (m_followSpeed > 0.0f) {
        float t = std::min(1.0f, m_followSpeed * deltaTime);
        m_position.x += (m_targetPosition.x - m_position.x) * t;
        m_position.y += (m_targetPosition.y - m_position.y) * t;
    } else {
        m_position = m_targetPosition;
    }
    
    // Apply bounds if set
    if (m_hasBounds) {
        applyBounds();
    }
    
    // Update screen shake
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= deltaTime;
        
        if (m_shakeTimer > 0.0f) {
            // Random offset based on intensity
            float angle = static_cast<float>(std::rand()) / RAND_MAX * 6.28318f;
            float distance = m_shakeIntensity * (m_shakeTimer / m_shakeDuration);
            m_shakeOffset.x = std::cos(angle) * distance;
            m_shakeOffset.y = std::sin(angle) * distance;
        } else {
            m_shakeOffset = Vector2(0, 0);
            m_shakeTimer = 0.0f;
        }
    }
}

void Camera::setPosition(const Vector2& pos) {
    m_position = pos;
    m_targetPosition = pos;
}

void Camera::follow(const Vector2& targetPos) {
    m_targetPosition = targetPos;
}

void Camera::setZoom(float zoom) {
    m_zoom = std::max(0.1f, std::min(10.0f, zoom)); // Clamp between 0.1 and 10
}

void Camera::zoomIn(float amount) {
    setZoom(m_zoom + amount);
}

void Camera::zoomOut(float amount) {
    setZoom(m_zoom - amount);
}

void Camera::setBounds(float minX, float minY, float maxX, float maxY) {
    m_hasBounds = true;
    m_minX = minX;
    m_minY = minY;
    m_maxX = maxX;
    m_maxY = maxY;
    applyBounds();
}

void Camera::clearBounds() {
    m_hasBounds = false;
}

void Camera::shake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeTimer = duration;
}

Vector2 Camera::screenToWorld(const Vector2& screenPos) const {
    Vector2 offset = getViewOffset();
    return Vector2(
        (screenPos.x / m_zoom) + offset.x,
        (screenPos.y / m_zoom) + offset.y
    );
}

Vector2 Camera::worldToScreen(const Vector2& worldPos) const {
    Vector2 offset = getViewOffset();
    return Vector2(
        (worldPos.x - offset.x) * m_zoom,
        (worldPos.y - offset.y) * m_zoom
    );
}

Vector2 Camera::getViewOffset() const {
    // Calculate top-left corner of camera view
    Vector2 finalPos = m_position;
    finalPos.x += m_shakeOffset.x;
    finalPos.y += m_shakeOffset.y;
    
    return Vector2(
        finalPos.x - (m_screenWidth / 2.0f) / m_zoom,
        finalPos.y - (m_screenHeight / 2.0f) / m_zoom
    );
}

void Camera::reset() {
    m_position = Vector2(m_screenWidth / 2.0f, m_screenHeight / 2.0f);
    m_targetPosition = m_position;
    m_zoom = 1.0f;
    m_shakeOffset = Vector2(0, 0);
    m_shakeTimer = 0.0f;
}

void Camera::applyBounds() {
    float halfWidth = (m_screenWidth / 2.0f) / m_zoom;
    float halfHeight = (m_screenHeight / 2.0f) / m_zoom;
    
    // Clamp camera position to bounds
    m_position.x = std::max(m_minX + halfWidth, std::min(m_maxX - halfWidth, m_position.x));
    m_position.y = std::max(m_minY + halfHeight, std::min(m_maxY - halfHeight, m_position.y));
    
    m_targetPosition.x = std::max(m_minX + halfWidth, std::min(m_maxX - halfWidth, m_targetPosition.x));
    m_targetPosition.y = std::max(m_minY + halfHeight, std::min(m_maxY - halfHeight, m_targetPosition.y));
}
