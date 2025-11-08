#ifndef OMEGA_UI_H
#define OMEGA_UI_H

#include "Sprite.h"
#include "Input.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Forward declaration
class UIManager;

// Base UI element
class UIElement {
public:
    UIElement();
    virtual ~UIElement() = default;
    
    virtual void update(float deltaTime) {}
    virtual void render(Shader* shader, int screenWidth, int screenHeight) = 0;
    virtual bool handleInput(Input& input) { return false; }
    
    // Properties
    void setPosition(const Vector2& pos) { m_position = pos; }
    void setSize(const Vector2& size) { m_size = size; }
    void setVisible(bool visible) { m_visible = visible; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    Vector2 getPosition() const { return m_position; }
    Vector2 getSize() const { return m_size; }
    bool isVisible() const { return m_visible; }
    bool isEnabled() const { return m_enabled; }
    
    // Hit testing
    bool contains(const Vector2& point) const;

protected:
    Vector2 m_position;
    Vector2 m_size;
    bool m_visible;
    bool m_enabled;
};

// UI Label (text display)
class UILabel : public UIElement {
public:
    UILabel(const std::string& text = "");
    ~UILabel() override = default;
    
    void setText(const std::string& text) { m_text = text; }
    void setColor(const Color& color) { m_color = color; }
    void setFontSize(float size) { m_fontSize = size; }
    
    std::string getText() const { return m_text; }
    Color getColor() const { return m_color; }
    
    void render(Shader* shader, int screenWidth, int screenHeight) override;

private:
    std::string m_text;
    Color m_color;
    float m_fontSize;
};

// UI Button
class UIButton : public UIElement {
public:
    UIButton(const std::string& text = "");
    ~UIButton() override = default;
    
    void setText(const std::string& text) { m_label.setText(text); }
    void setNormalColor(const Color& color) { m_normalColor = color; }
    void setHoverColor(const Color& color) { m_hoverColor = color; }
    void setPressedColor(const Color& color) { m_pressedColor = color; }
    void setTextColor(const Color& color) { m_label.setColor(color); }
    
    void setOnClick(std::function<void()> callback) { m_onClick = callback; }
    
    void update(float deltaTime) override;
    bool handleInput(Input& input) override;
    void render(Shader* shader, int screenWidth, int screenHeight) override;

private:
    UILabel m_label;
    Color m_normalColor;
    Color m_hoverColor;
    Color m_pressedColor;
    Color m_currentColor;
    bool m_hovered;
    bool m_pressed;
    std::function<void()> m_onClick;
};

// UI Panel (container)
class UIPanel : public UIElement {
public:
    UIPanel();
    ~UIPanel() override = default;
    
    void setBackgroundColor(const Color& color) { m_backgroundColor = color; }
    void setBorderColor(const Color& color) { m_borderColor = color; }
    void setBorderWidth(float width) { m_borderWidth = width; }
    
    Color getBackgroundColor() const { return m_backgroundColor; }
    
    void render(Shader* shader, int screenWidth, int screenHeight) override;

private:
    Color m_backgroundColor;
    Color m_borderColor;
    float m_borderWidth;
};

// UI Manager
class UIManager {
public:
    UIManager();
    ~UIManager() = default;
    
    // Element management
    template<typename T, typename... Args>
    T* addElement(Args&&... args) {
        auto element = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = element.get();
        m_elements.push_back(std::move(element));
        return ptr;
    }
    
    void clear();
    
    // Update/render all elements
    void update(float deltaTime);
    void handleInput(Input& input);
    void render(Shader* shader, int screenWidth, int screenHeight);
    
    // Getters
    size_t getElementCount() const { return m_elements.size(); }

private:
    std::vector<std::unique_ptr<UIElement>> m_elements;
};

#endif // OMEGA_UI_H
