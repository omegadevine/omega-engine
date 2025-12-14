/**
 * OMEGA ENGINE - Input System
 * 
 * Handles keyboard, mouse, and gamepad input for all Omega games.
 * Provides both polling and event-based input checking.
 */

#ifndef OMEGA_INPUT_H
#define OMEGA_INPUT_H

#include <SDL2/SDL.h>
#include <unordered_set>
#include "OmegaMath.h"

namespace Omega {

/**
 * Key codes - maps to SDL scancodes for common keys
 */
enum class Key {
    // Letters
    A = SDL_SCANCODE_A, B = SDL_SCANCODE_B, C = SDL_SCANCODE_C,
    D = SDL_SCANCODE_D, E = SDL_SCANCODE_E, F = SDL_SCANCODE_F,
    G = SDL_SCANCODE_G, H = SDL_SCANCODE_H, I = SDL_SCANCODE_I,
    J = SDL_SCANCODE_J, K = SDL_SCANCODE_K, L = SDL_SCANCODE_L,
    M = SDL_SCANCODE_M, N = SDL_SCANCODE_N, O = SDL_SCANCODE_O,
    P = SDL_SCANCODE_P, Q = SDL_SCANCODE_Q, R = SDL_SCANCODE_R,
    S = SDL_SCANCODE_S, T = SDL_SCANCODE_T, U = SDL_SCANCODE_U,
    V = SDL_SCANCODE_V, W = SDL_SCANCODE_W, X = SDL_SCANCODE_X,
    Y = SDL_SCANCODE_Y, Z = SDL_SCANCODE_Z,
    
    // Numbers
    Num0 = SDL_SCANCODE_0, Num1 = SDL_SCANCODE_1, Num2 = SDL_SCANCODE_2,
    Num3 = SDL_SCANCODE_3, Num4 = SDL_SCANCODE_4, Num5 = SDL_SCANCODE_5,
    Num6 = SDL_SCANCODE_6, Num7 = SDL_SCANCODE_7, Num8 = SDL_SCANCODE_8,
    Num9 = SDL_SCANCODE_9,
    
    // Function keys
    F1 = SDL_SCANCODE_F1, F2 = SDL_SCANCODE_F2, F3 = SDL_SCANCODE_F3,
    F4 = SDL_SCANCODE_F4, F5 = SDL_SCANCODE_F5, F6 = SDL_SCANCODE_F6,
    F7 = SDL_SCANCODE_F7, F8 = SDL_SCANCODE_F8, F9 = SDL_SCANCODE_F9,
    F10 = SDL_SCANCODE_F10, F11 = SDL_SCANCODE_F11, F12 = SDL_SCANCODE_F12,
    
    // Arrow keys
    Up = SDL_SCANCODE_UP, Down = SDL_SCANCODE_DOWN,
    Left = SDL_SCANCODE_LEFT, Right = SDL_SCANCODE_RIGHT,
    
    // Modifiers
    LShift = SDL_SCANCODE_LSHIFT, RShift = SDL_SCANCODE_RSHIFT,
    LCtrl = SDL_SCANCODE_LCTRL, RCtrl = SDL_SCANCODE_RCTRL,
    LAlt = SDL_SCANCODE_LALT, RAlt = SDL_SCANCODE_RALT,
    
    // Common keys
    Space = SDL_SCANCODE_SPACE,
    Enter = SDL_SCANCODE_RETURN,
    Escape = SDL_SCANCODE_ESCAPE,
    Tab = SDL_SCANCODE_TAB,
    Backspace = SDL_SCANCODE_BACKSPACE,
    Delete = SDL_SCANCODE_DELETE,
    Insert = SDL_SCANCODE_INSERT,
    Home = SDL_SCANCODE_HOME,
    End = SDL_SCANCODE_END,
    PageUp = SDL_SCANCODE_PAGEUP,
    PageDown = SDL_SCANCODE_PAGEDOWN,
    
    // Numpad
    KP0 = SDL_SCANCODE_KP_0, KP1 = SDL_SCANCODE_KP_1, KP2 = SDL_SCANCODE_KP_2,
    KP3 = SDL_SCANCODE_KP_3, KP4 = SDL_SCANCODE_KP_4, KP5 = SDL_SCANCODE_KP_5,
    KP6 = SDL_SCANCODE_KP_6, KP7 = SDL_SCANCODE_KP_7, KP8 = SDL_SCANCODE_KP_8,
    KP9 = SDL_SCANCODE_KP_9,
    KPPlus = SDL_SCANCODE_KP_PLUS, KPMinus = SDL_SCANCODE_KP_MINUS,
    KPMultiply = SDL_SCANCODE_KP_MULTIPLY, KPDivide = SDL_SCANCODE_KP_DIVIDE,
    KPEnter = SDL_SCANCODE_KP_ENTER
};

/**
 * Mouse buttons
 */
enum class MouseButton {
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2
};

/**
 * Gamepad buttons (Xbox-style layout)
 */
enum class GamepadButton {
    A = SDL_CONTROLLER_BUTTON_A,
    B = SDL_CONTROLLER_BUTTON_B,
    X = SDL_CONTROLLER_BUTTON_X,
    Y = SDL_CONTROLLER_BUTTON_Y,
    Back = SDL_CONTROLLER_BUTTON_BACK,
    Guide = SDL_CONTROLLER_BUTTON_GUIDE,
    Start = SDL_CONTROLLER_BUTTON_START,
    LeftStick = SDL_CONTROLLER_BUTTON_LEFTSTICK,
    RightStick = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,
    DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

/**
 * Gamepad axes
 */
enum class GamepadAxis {
    LeftX = SDL_CONTROLLER_AXIS_LEFTX,
    LeftY = SDL_CONTROLLER_AXIS_LEFTY,
    RightX = SDL_CONTROLLER_AXIS_RIGHTX,
    RightY = SDL_CONTROLLER_AXIS_RIGHTY,
    TriggerLeft = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    TriggerRight = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
};

/**
 * Main input class
 */
class Input {
    friend class Core;
private:
    // Keyboard state
    const uint8_t* keyboardState;
    std::unordered_set<int> keysPressed;    // Keys pressed this frame
    std::unordered_set<int> keysReleased;   // Keys released this frame
    
    // Mouse state
    Vec2i mousePosition;
    Vec2i mouseDelta;
    uint32_t mouseButtons;
    uint32_t prevMouseButtons;
    int mouseWheelDelta;
    
    // Gamepad state
    SDL_GameController* gamepad;
    bool gamepadConnected;
    
    // Private constructor
    Input();
    
    bool init();
    void shutdown();
    void update();  // Called at start of frame
    void processEvent(const SDL_Event& event);
    
public:
    ~Input();
    
    // ========================================
    // KEYBOARD
    // ========================================
    
    /**
     * Check if a key is currently held down
     */
    bool isKeyDown(Key key) const;
    
    /**
     * Check if a key was just pressed this frame
     */
    bool isKeyPressed(Key key) const;
    
    /**
     * Check if a key was just released this frame
     */
    bool isKeyReleased(Key key) const;
    
    /**
     * Check if any key is pressed
     */
    bool isAnyKeyPressed() const;
    
    /**
     * Check modifier keys
     */
    bool isShiftDown() const;
    bool isCtrlDown() const;
    bool isAltDown() const;
    
    // ========================================
    // MOUSE
    // ========================================
    
    /**
     * Get mouse position (screen coordinates)
     */
    Vec2i getMousePosition() const { return mousePosition; }
    int getMouseX() const { return mousePosition.x; }
    int getMouseY() const { return mousePosition.y; }
    
    /**
     * Get mouse movement since last frame
     */
    Vec2i getMouseDelta() const { return mouseDelta; }
    
    /**
     * Check if a mouse button is held down
     */
    bool isMouseButtonDown(MouseButton button) const;
    
    /**
     * Check if a mouse button was just pressed
     */
    bool isMouseButtonPressed(MouseButton button) const;
    
    /**
     * Check if a mouse button was just released
     */
    bool isMouseButtonReleased(MouseButton button) const;
    
    /**
     * Get mouse wheel scroll delta (positive = up, negative = down)
     */
    int getMouseWheelDelta() const { return mouseWheelDelta; }
    
    // ========================================
    // GAMEPAD
    // ========================================
    
    /**
     * Manually scan for and connect a gamepad
     * (Called automatically when controller is plugged in via events)
     */
    void tryConnectGamepad();
    
    /**
     * Check if a gamepad is connected
     */
    bool isGamepadConnected() const { return gamepadConnected; }
    
    /**
     * Check if a gamepad button is held down
     */
    bool isGamepadButtonDown(GamepadButton button) const;
    
    /**
     * Get gamepad axis value (-1.0 to 1.0)
     */
    float getGamepadAxis(GamepadAxis axis) const;
    
    /**
     * Get left stick as vector
     */
    Vec2 getLeftStick() const;
    
    /**
     * Get right stick as vector
     */
    Vec2 getRightStick() const;
    
    /**
     * Get trigger values (0.0 to 1.0)
     */
    float getLeftTrigger() const;
    float getRightTrigger() const;
    
    // ========================================
    // UTILITY
    // ========================================
    
    /**
     * Get horizontal axis input (-1, 0, or 1) from arrows/WASD/gamepad
     */
    int getHorizontalAxis() const;
    
    /**
     * Get vertical axis input (-1, 0, or 1) from arrows/WASD/gamepad
     */
    int getVerticalAxis() const;
    
    /**
     * Get movement vector from all input sources (normalized)
     */
    Vec2 getMovementVector() const;
};

} // namespace Omega

#endif // OMEGA_INPUT_H
