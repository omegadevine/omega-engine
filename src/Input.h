#ifndef OMEGA_INPUT_H
#define OMEGA_INPUT_H

#include <SDL.h>
#include <unordered_map>

// Forward declare Vector2
struct Vector2;

enum class KeyCode {
    W, A, S, D,
    Up, Down, Left, Right,
    Space, Escape, Enter,
    Unknown
};

enum class MouseButton {
    Left, Right, Middle
};

class Input {
public:
    static Input& getInstance();

    void update(const SDL_Event& event);
    void endFrame();

    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    bool isKeyReleased(KeyCode key) const;

    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonJustPressed(MouseButton button) const;
    bool isMouseButtonJustReleased(MouseButton button) const;

    int getMouseX() const { return m_mouseX; }
    int getMouseY() const { return m_mouseY; }
    Vector2 getMousePosition() const { return Vector2(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY)); }

private:
    Input();
    ~Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    KeyCode sdlKeyToKeyCode(SDL_Keycode key);
    int mouseButtonToInt(MouseButton button) const;

    std::unordered_map<KeyCode, bool> m_keyStates;
    std::unordered_map<KeyCode, bool> m_keyJustPressed;
    std::unordered_map<MouseButton, bool> m_mouseStates;
    std::unordered_map<MouseButton, bool> m_mouseJustPressed;
    std::unordered_map<MouseButton, bool> m_mouseJustReleased;

    int m_mouseX;
    int m_mouseY;
};

#endif // OMEGA_INPUT_H
