/**
 * OMEGA ENGINE - Input System Implementation
 */

#include "OmegaInput.h"
#include <cmath>

namespace Omega {

Input::Input() 
    : keyboardState(nullptr), mousePosition(0, 0), mouseDelta(0, 0),
      mouseButtons(0), prevMouseButtons(0), mouseWheelDelta(0),
      gamepad(nullptr), gamepadConnected(false) {
}

Input::~Input() {
    shutdown();
}

bool Input::init() {
    keyboardState = SDL_GetKeyboardState(nullptr);
    
    // Don't auto-scan for gamepads at startup to avoid AV false positives
    // Gamepad will be detected when plugged in via SDL events
    gamepad = nullptr;
    gamepadConnected = false;
    
    return true;
}

void Input::tryConnectGamepad() {
    // Manual gamepad connection - call this if you want to scan for gamepads
    if (gamepadConnected) return;
    
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gamepad = SDL_GameControllerOpen(i);
            if (gamepad) {
                gamepadConnected = true;
                break;
            }
        }
    }
}

void Input::shutdown() {
    if (gamepad) {
        SDL_GameControllerClose(gamepad);
        gamepad = nullptr;
        gamepadConnected = false;
    }
}

void Input::update() {
    // Clear per-frame state
    keysPressed.clear();
    keysReleased.clear();
    mouseWheelDelta = 0;
    
    // Store previous mouse state
    prevMouseButtons = mouseButtons;
    
    // Get current mouse state
    Vec2i prevPos = mousePosition;
    mouseButtons = SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
    mouseDelta = mousePosition - prevPos;
}

void Input::processEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            if (!event.key.repeat) {
                keysPressed.insert(event.key.keysym.scancode);
            }
            break;
            
        case SDL_KEYUP:
            keysReleased.insert(event.key.keysym.scancode);
            break;
            
        case SDL_MOUSEWHEEL:
            mouseWheelDelta += event.wheel.y;
            break;
            
        case SDL_CONTROLLERDEVICEADDED:
            if (!gamepadConnected) {
                gamepad = SDL_GameControllerOpen(event.cdevice.which);
                if (gamepad) {
                    gamepadConnected = true;
                }
            }
            break;
            
        case SDL_CONTROLLERDEVICEREMOVED:
            if (gamepad && event.cdevice.which == SDL_JoystickInstanceID(
                    SDL_GameControllerGetJoystick(gamepad))) {
                SDL_GameControllerClose(gamepad);
                gamepad = nullptr;
                gamepadConnected = false;
            }
            break;
    }
}

// ========================================
// KEYBOARD
// ========================================

bool Input::isKeyDown(Key key) const {
    return keyboardState && keyboardState[static_cast<int>(key)];
}

bool Input::isKeyPressed(Key key) const {
    return keysPressed.count(static_cast<int>(key)) > 0;
}

bool Input::isKeyReleased(Key key) const {
    return keysReleased.count(static_cast<int>(key)) > 0;
}

bool Input::isAnyKeyPressed() const {
    return !keysPressed.empty();
}

bool Input::isShiftDown() const {
    return isKeyDown(Key::LShift) || isKeyDown(Key::RShift);
}

bool Input::isCtrlDown() const {
    return isKeyDown(Key::LCtrl) || isKeyDown(Key::RCtrl);
}

bool Input::isAltDown() const {
    return isKeyDown(Key::LAlt) || isKeyDown(Key::RAlt);
}

// ========================================
// MOUSE
// ========================================

bool Input::isMouseButtonDown(MouseButton button) const {
    return mouseButtons & SDL_BUTTON(static_cast<int>(button));
}

bool Input::isMouseButtonPressed(MouseButton button) const {
    uint32_t mask = SDL_BUTTON(static_cast<int>(button));
    return (mouseButtons & mask) && !(prevMouseButtons & mask);
}

bool Input::isMouseButtonReleased(MouseButton button) const {
    uint32_t mask = SDL_BUTTON(static_cast<int>(button));
    return !(mouseButtons & mask) && (prevMouseButtons & mask);
}

// ========================================
// GAMEPAD
// ========================================

bool Input::isGamepadButtonDown(GamepadButton button) const {
    if (!gamepad) return false;
    return SDL_GameControllerGetButton(gamepad, 
        static_cast<SDL_GameControllerButton>(button));
}

float Input::getGamepadAxis(GamepadAxis axis) const {
    if (!gamepad) return 0.0f;
    int16_t value = SDL_GameControllerGetAxis(gamepad,
        static_cast<SDL_GameControllerAxis>(axis));
    return value / 32767.0f;
}

Vec2 Input::getLeftStick() const {
    return Vec2(getGamepadAxis(GamepadAxis::LeftX),
                getGamepadAxis(GamepadAxis::LeftY));
}

Vec2 Input::getRightStick() const {
    return Vec2(getGamepadAxis(GamepadAxis::RightX),
                getGamepadAxis(GamepadAxis::RightY));
}

float Input::getLeftTrigger() const {
    float val = getGamepadAxis(GamepadAxis::TriggerLeft);
    return val > 0 ? val : 0;
}

float Input::getRightTrigger() const {
    float val = getGamepadAxis(GamepadAxis::TriggerRight);
    return val > 0 ? val : 0;
}

// ========================================
// UTILITY
// ========================================

int Input::getHorizontalAxis() const {
    int axis = 0;
    
    // Keyboard
    if (isKeyDown(Key::Left) || isKeyDown(Key::A)) axis -= 1;
    if (isKeyDown(Key::Right) || isKeyDown(Key::D)) axis += 1;
    
    // Gamepad
    if (gamepadConnected) {
        float stickX = getGamepadAxis(GamepadAxis::LeftX);
        if (stickX < -0.3f) axis -= 1;
        if (stickX > 0.3f) axis += 1;
        
        if (isGamepadButtonDown(GamepadButton::DPadLeft)) axis -= 1;
        if (isGamepadButtonDown(GamepadButton::DPadRight)) axis += 1;
    }
    
    return clampi(axis, -1, 1);
}

int Input::getVerticalAxis() const {
    int axis = 0;
    
    // Keyboard
    if (isKeyDown(Key::Up) || isKeyDown(Key::W)) axis -= 1;
    if (isKeyDown(Key::Down) || isKeyDown(Key::S)) axis += 1;
    
    // Gamepad
    if (gamepadConnected) {
        float stickY = getGamepadAxis(GamepadAxis::LeftY);
        if (stickY < -0.3f) axis -= 1;
        if (stickY > 0.3f) axis += 1;
        
        if (isGamepadButtonDown(GamepadButton::DPadUp)) axis -= 1;
        if (isGamepadButtonDown(GamepadButton::DPadDown)) axis += 1;
    }
    
    return clampi(axis, -1, 1);
}

Vec2 Input::getMovementVector() const {
    Vec2 movement;
    
    // Keyboard - digital input
    if (isKeyDown(Key::Left) || isKeyDown(Key::A)) movement.x -= 1;
    if (isKeyDown(Key::Right) || isKeyDown(Key::D)) movement.x += 1;
    if (isKeyDown(Key::Up) || isKeyDown(Key::W)) movement.y -= 1;
    if (isKeyDown(Key::Down) || isKeyDown(Key::S)) movement.y += 1;
    
    // Gamepad - analog input (takes priority if significant)
    if (gamepadConnected) {
        Vec2 stick = getLeftStick();
        if (stick.length() > 0.3) {
            movement = stick;
        }
    }
    
    // Normalize if length > 1
    if (movement.length() > 1.0) {
        movement = movement.normalized();
    }
    
    return movement;
}

} // namespace Omega
