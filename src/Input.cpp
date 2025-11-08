#include "Input.h"
#include "Sprite.h"  // For Vector2

Input& Input::getInstance() {
    static Input instance;
    return instance;
}

Input::Input() : m_mouseX(0), m_mouseY(0) {
}

KeyCode Input::sdlKeyToKeyCode(SDL_Keycode key) {
    switch (key) {
        case SDLK_w: return KeyCode::W;
        case SDLK_a: return KeyCode::A;
        case SDLK_s: return KeyCode::S;
        case SDLK_d: return KeyCode::D;
        case SDLK_UP: return KeyCode::Up;
        case SDLK_DOWN: return KeyCode::Down;
        case SDLK_LEFT: return KeyCode::Left;
        case SDLK_RIGHT: return KeyCode::Right;
        case SDLK_SPACE: return KeyCode::Space;
        case SDLK_ESCAPE: return KeyCode::Escape;
        case SDLK_RETURN: return KeyCode::Enter;
        default: return KeyCode::Unknown;
    }
}

int Input::mouseButtonToInt(MouseButton button) const {
    switch (button) {
        case MouseButton::Left: return SDL_BUTTON_LEFT;
        case MouseButton::Right: return SDL_BUTTON_RIGHT;
        case MouseButton::Middle: return SDL_BUTTON_MIDDLE;
        default: return -1;
    }
}

void Input::update(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        KeyCode key = sdlKeyToKeyCode(event.key.keysym.sym);
        if (key != KeyCode::Unknown) {
            if (!m_keyStates[key]) {
                m_keyJustPressed[key] = true;
            }
            m_keyStates[key] = true;
        }
    }
    else if (event.type == SDL_KEYUP) {
        KeyCode key = sdlKeyToKeyCode(event.key.keysym.sym);
        if (key != KeyCode::Unknown) {
            m_keyStates[key] = false;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (!m_mouseStates[MouseButton::Left]) {
                m_mouseJustPressed[MouseButton::Left] = true;
            }
            m_mouseStates[MouseButton::Left] = true;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
            if (!m_mouseStates[MouseButton::Right]) {
                m_mouseJustPressed[MouseButton::Right] = true;
            }
            m_mouseStates[MouseButton::Right] = true;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            m_mouseStates[MouseButton::Left] = false;
            m_mouseJustReleased[MouseButton::Left] = true;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
            m_mouseStates[MouseButton::Right] = false;
            m_mouseJustReleased[MouseButton::Right] = true;
        }
    }
    else if (event.type == SDL_MOUSEMOTION) {
        m_mouseX = event.motion.x;
        m_mouseY = event.motion.y;
    }
}

void Input::endFrame() {
    m_keyJustPressed.clear();
    m_mouseJustPressed.clear();
    m_mouseJustReleased.clear();
}

bool Input::isKeyPressed(KeyCode key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second;
}

bool Input::isKeyJustPressed(KeyCode key) const {
    auto it = m_keyJustPressed.find(key);
    return it != m_keyJustPressed.end() && it->second;
}

bool Input::isKeyReleased(KeyCode key) const {
    auto it = m_keyStates.find(key);
    return it == m_keyStates.end() || !it->second;
}

bool Input::isMouseButtonPressed(MouseButton button) const {
    auto it = m_mouseStates.find(button);
    return it != m_mouseStates.end() && it->second;
}

bool Input::isMouseButtonJustPressed(MouseButton button) const {
    auto it = m_mouseJustPressed.find(button);
    return it != m_mouseJustPressed.end() && it->second;
}

bool Input::isMouseButtonJustReleased(MouseButton button) const {
    auto it = m_mouseJustReleased.find(button);
    return it != m_mouseJustReleased.end() && it->second;
}
