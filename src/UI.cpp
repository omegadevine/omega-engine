#include "UI.h"
#include <iostream>

// ============================================================================
// UIElement Implementation
// ============================================================================

UIElement::UIElement()
    : m_position(0, 0)
    , m_size(100, 50)
    , m_visible(true)
    , m_enabled(true) {
}

bool UIElement::contains(const Vector2& point) const {
    return point.x >= m_position.x && 
           point.x <= m_position.x + m_size.x &&
           point.y >= m_position.y && 
           point.y <= m_position.y + m_size.y;
}

// ============================================================================
// UILabel Implementation
// ============================================================================

UILabel::UILabel(const std::string& text)
    : UIElement()
    , m_text(text)
    , m_color(1.0f, 1.0f, 1.0f, 1.0f)
    , m_fontSize(16.0f) {
}

void UILabel::render(Shader* shader, int screenWidth, int screenHeight) {
    if (!m_visible) return;
    
    // For now, we'll render text as colored rectangles (placeholder)
    // In a real implementation, you'd use a font rendering library like FreeType
    // or render bitmap fonts
    
    // Simple text representation - one rectangle per character
    float charWidth = m_fontSize * 0.6f;
    float charHeight = m_fontSize;
    
    Sprite charSprite;
    charSprite.setSize(Vector2(charWidth, charHeight));
    charSprite.setColor(m_color);
    
    for (size_t i = 0; i < m_text.length(); i++) {
        Vector2 charPos = m_position;
        charPos.x += i * (charWidth + 2.0f);
        charSprite.setPosition(charPos);
        charSprite.draw(shader, screenWidth, screenHeight);
    }
}

// ============================================================================
// UIButton Implementation
// ============================================================================

UIButton::UIButton(const std::string& text)
    : UIElement()
    , m_label(text)
    , m_normalColor(0.3f, 0.3f, 0.4f, 1.0f)
    , m_hoverColor(0.4f, 0.4f, 0.5f, 1.0f)
    , m_pressedColor(0.2f, 0.2f, 0.3f, 1.0f)
    , m_currentColor(0.3f, 0.3f, 0.4f, 1.0f)
    , m_hovered(false)
    , m_pressed(false)
    , m_onClick(nullptr) {
    
    m_size = Vector2(200, 50);
    m_label.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void UIButton::update(float deltaTime) {
    if (!m_visible || !m_enabled) {
        m_currentColor = m_normalColor;
        return;
    }
    
    // Smooth color transitions
    Color targetColor = m_pressed ? m_pressedColor : (m_hovered ? m_hoverColor : m_normalColor);
    
    float lerpSpeed = 10.0f * deltaTime;
    m_currentColor.r += (targetColor.r - m_currentColor.r) * lerpSpeed;
    m_currentColor.g += (targetColor.g - m_currentColor.g) * lerpSpeed;
    m_currentColor.b += (targetColor.b - m_currentColor.b) * lerpSpeed;
}

bool UIButton::handleInput(Input& input) {
    if (!m_visible || !m_enabled) {
        m_hovered = false;
        m_pressed = false;
        return false;
    }
    
    Vector2 mousePos = input.getMousePosition();
    m_hovered = contains(mousePos);
    
    if (m_hovered) {
        if (input.isMouseButtonPressed(MouseButton::Left)) {
            m_pressed = true;
        } else if (m_pressed && input.isMouseButtonJustReleased(MouseButton::Left)) {
            m_pressed = false;
            if (m_onClick) {
                m_onClick();
            }
            return true; // Consumed input
        } else {
            m_pressed = false;
        }
    } else {
        m_pressed = false;
    }
    
    return m_hovered; // Return true if mouse is over button
}

void UIButton::render(Shader* shader, int screenWidth, int screenHeight) {
    if (!m_visible) return;
    
    // Render button background
    Sprite buttonSprite;
    buttonSprite.setPosition(m_position);
    buttonSprite.setSize(m_size);
    buttonSprite.setColor(m_currentColor);
    buttonSprite.draw(shader, screenWidth, screenHeight);
    
    // Render button border
    float borderWidth = 2.0f;
    Color borderColor = m_hovered ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(0.5f, 0.5f, 0.5f, 1.0f);
    
    // Top border
    Sprite border;
    border.setColor(borderColor);
    border.setPosition(m_position);
    border.setSize(Vector2(m_size.x, borderWidth));
    border.draw(shader, screenWidth, screenHeight);
    
    // Bottom border
    border.setPosition(Vector2(m_position.x, m_position.y + m_size.y - borderWidth));
    border.draw(shader, screenWidth, screenHeight);
    
    // Left border
    border.setPosition(m_position);
    border.setSize(Vector2(borderWidth, m_size.y));
    border.draw(shader, screenWidth, screenHeight);
    
    // Right border
    border.setPosition(Vector2(m_position.x + m_size.x - borderWidth, m_position.y));
    border.draw(shader, screenWidth, screenHeight);
    
    // Render label centered
    Vector2 labelPos = m_position;
    labelPos.x += (m_size.x - m_label.getText().length() * 10.0f) * 0.5f;
    labelPos.y += (m_size.y - 16.0f) * 0.5f;
    m_label.setPosition(labelPos);
    m_label.render(shader, screenWidth, screenHeight);
}

// ============================================================================
// UIPanel Implementation
// ============================================================================

UIPanel::UIPanel()
    : UIElement()
    , m_backgroundColor(0.2f, 0.2f, 0.25f, 0.9f)
    , m_borderColor(0.4f, 0.4f, 0.45f, 1.0f)
    , m_borderWidth(2.0f) {
    
    m_size = Vector2(400, 300);
}

void UIPanel::render(Shader* shader, int screenWidth, int screenHeight) {
    if (!m_visible) return;
    
    // Render panel background
    Sprite panelSprite;
    panelSprite.setPosition(m_position);
    panelSprite.setSize(m_size);
    panelSprite.setColor(m_backgroundColor);
    panelSprite.draw(shader, screenWidth, screenHeight);
    
    // Render border if width > 0
    if (m_borderWidth > 0.0f) {
        Sprite border;
        border.setColor(m_borderColor);
        
        // Top border
        border.setPosition(m_position);
        border.setSize(Vector2(m_size.x, m_borderWidth));
        border.draw(shader, screenWidth, screenHeight);
        
        // Bottom border
        border.setPosition(Vector2(m_position.x, m_position.y + m_size.y - m_borderWidth));
        border.draw(shader, screenWidth, screenHeight);
        
        // Left border
        border.setPosition(m_position);
        border.setSize(Vector2(m_borderWidth, m_size.y));
        border.draw(shader, screenWidth, screenHeight);
        
        // Right border
        border.setPosition(Vector2(m_position.x + m_size.x - m_borderWidth, m_position.y));
        border.draw(shader, screenWidth, screenHeight);
    }
}

// ============================================================================
// UIManager Implementation
// ============================================================================

UIManager::UIManager() {
}

void UIManager::clear() {
    m_elements.clear();
}

void UIManager::update(float deltaTime) {
    for (auto& element : m_elements) {
        if (element->isVisible()) {
            element->update(deltaTime);
        }
    }
}

void UIManager::handleInput(Input& input) {
    // Process in reverse order (front to back)
    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if ((*it)->isVisible() && (*it)->isEnabled()) {
            if ((*it)->handleInput(input)) {
                break; // Stop if element consumed input
            }
        }
    }
}

void UIManager::render(Shader* shader, int screenWidth, int screenHeight) {
    for (auto& element : m_elements) {
        if (element->isVisible()) {
            element->render(shader, screenWidth, screenHeight);
        }
    }
}
