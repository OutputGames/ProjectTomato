/**
 * @file input.hpp
 * @brief Input handling for mouse, keyboard, and gamepad
 * 
 * This header provides a unified input system that supports:
 * - Mouse position, movement, and button states
 * - Keyboard key states (press, hold, release)
 * - Gamepad button and axis input
 * - Virtual axes for cross-platform input (e.g., "Horizontal", "Vertical")
 * - Automatic switching between keyboard/mouse and gamepad input modes
 */

#ifndef INPUT_H
#define INPUT_H

#include "utils.hpp"


namespace tmt::render
{
    struct Camera;
}

namespace tmt::input
{

    struct Mouse;
    struct Keyboard;

    /**
     * @brief Mouse input handling
     * 
     * Provides static methods to query mouse state including:
     * - Screen position and movement delta
     * - Scroll wheel delta
     * - Button states (left, middle, right)
     * - World-space mouse position via raycasting
     */
    struct Mouse
    {
        /**
         * @brief Get current mouse position in screen coordinates
         * @return glm::vec2 Mouse position (x, y) in pixels from top-left
         */
        static glm::vec2 GetMousePosition();
        
        /**
         * @brief Get mouse movement delta since last frame
         * @return glm::vec2 Mouse movement (x, y) in pixels
         */
        static glm::vec2 GetMouseDelta();
        
        /**
         * @brief Get mouse scroll wheel delta
         * @return glm::vec2 Scroll delta (typically only y is used)
         */
        static glm::vec2 GetMouseScroll();
        
        /**
         * @brief Get mouse position in world space using raycasting
         * 
         * Casts a ray from the camera through the mouse cursor and returns
         * the first intersection point with the physics world.
         * 
         * @param camera Camera to use for raycasting
         * @return glm::vec3 World position where mouse ray hits, or (0,0,0) if no hit
         */
        static glm::vec3 GetWorldMousePosition(render::Camera* camera);

        /**
         * @brief Mouse button identifiers
         */
        enum MouseButton
        {
            Left   = GLFW_MOUSE_BUTTON_LEFT,
            Middle = GLFW_MOUSE_BUTTON_MIDDLE,
            Right  = GLFW_MOUSE_BUTTON_RIGHT
        };

        /**
         * @brief Mouse button state
         * 
         * - Release: Button is not pressed
         * - Press: Button was just pressed this frame
         * - Hold: Button is being held down (was pressed in previous frame too)
         */
        enum MouseButtonState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        /**
         * @brief Get the state of a mouse button
         * 
         * @param btn Button to query (Left, Middle, or Right)
         * @param real If true, gets raw state without UI filtering (default: false)
         * @return MouseButtonState Current state of the button
         */
        static MouseButtonState GetMouseButton(MouseButton btn, bool real = false);
    };

    /**
     * @brief Keyboard input handling
     * 
     * Provides static methods to query keyboard key states.
     * Uses GLFW key codes (e.g., GLFW_KEY_W, GLFW_KEY_SPACE).
     */
    struct Keyboard
    {
        /**
         * @brief Key state
         * 
         * - Release: Key is not pressed
         * - Press: Key was just pressed this frame
         * - Hold: Key is being held down
         */
        enum KeyState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        /**
         * @brief Get the state of a keyboard key
         * 
         * @param key GLFW key code (e.g., GLFW_KEY_W, GLFW_KEY_ESCAPE)
         * @return KeyState Current state of the key
         */
        static KeyState GetKey(int key);
    };

    /**
     * @brief Gamepad input handling
     * 
     * Provides static methods to query gamepad button and axis states.
     * Uses GLFW gamepad mappings for button and axis identifiers.
     */
    struct Gamepad
    {
        /**
         * @brief Gamepad button state
         * 
         * - Release: Button is not pressed
         * - Press: Button was just pressed this frame
         * - Hold: Button is being held down
         */
        enum PadState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        /**
         * @brief Get the state of a gamepad button
         * 
         * @param button GLFW gamepad button ID (e.g., GLFW_GAMEPAD_BUTTON_A)
         * @return PadState Current state of the button
         */
        static PadState GetButton(int button);
        
        /**
         * @brief Get the value of a gamepad axis
         * 
         * @param axis GLFW gamepad axis ID (e.g., GLFW_GAMEPAD_AXIS_LEFT_X)
         * @return float Axis value, typically in range [-1, 1] (deadzone applied)
         */
        static float GetAxis(int axis);
    };

    /**
     * @brief Active input method
     * 
     * Tracks which input method the player is currently using.
     * Can automatically switch based on the last input received.
     */
    enum InputState
    {
        None = 0,           ///< No input detected
        KeyboardMouse,      ///< Using keyboard and mouse
        Gamepad             ///< Using gamepad
    };

    /**
     * @brief Get the current active input method
     * @return InputState Current input state (KeyboardMouse or Gamepad)
     */
    InputState GetInputState();
    
    /**
     * @brief Force a specific input method
     * 
     * Prevents automatic switching between input methods.
     * 
     * @param state Input state to force
     */
    void ForceInputState(InputState state);

    /**
     * @brief Get a virtual axis value by name
     * 
     * Virtual axes provide consistent input across keyboard and gamepad.
     * Supported axes:
     * - "Horizontal": Left/Right or A/D keys, or left stick X
     * - "Vertical": Up/Down or W/S keys, or left stick Y
     * - "LookHorizontal": Mouse X delta or right stick X
     * - "LookVertical": Mouse Y delta or right stick Y
     * 
     * @param axis Name of the virtual axis
     * @return float Axis value in range [-1, 1]
     */
    float GetAxis(string axis);
    
    /**
     * @brief Get a 2D virtual axis by name
     * 
     * @param axis Name of the 2D axis (e.g., "Movement", "Look")
     * @return glm::vec2 2D axis value
     */
    glm::vec2 GetAxis2(string axis);

    /**
     * @brief Get the name of the connected gamepad
     * @return string Gamepad name, or empty if no gamepad connected
     */
    string GetGamepadName();
    
    /**
     * @brief Get the last key that was pressed
     * @return int GLFW key code of last pressed key
     */
    int GetLastKey();

    /**
     * @brief Update input state for this frame
     * 
     * Called once per frame by the engine to update key/button states
     * and detect Press/Hold/Release transitions.
     */
    void Update();
    
    /**
     * @brief Initialize the input system
     * 
     * Sets up input callbacks and initializes state tracking.
     * Called once during engine initialization.
     */
    void init();

}

#endif
