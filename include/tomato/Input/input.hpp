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

    struct Mouse
    {
        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseDelta();
        static glm::vec3 GetWorldMousePosition(render::Camera* camera);

        enum MouseButton
        {
            Left   = GLFW_MOUSE_BUTTON_LAST,
            Middle = GLFW_MOUSE_BUTTON_MIDDLE,
            Right  = GLFW_MOUSE_BUTTON_RIGHT
        };

        enum MouseButtonState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        static MouseButtonState GetMouseButton(MouseButton btn);
    };

    struct Keyboard
    {
        enum KeyState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        static KeyState GetKey(int key);
    };

    struct Gamepad
    {
        enum PadState
        {
            Release = GLFW_RELEASE,
            Press   = GLFW_PRESS,
            Hold,
        };

        static PadState GetButton(int button);
    };

    inline enum InputState
    {
        None = 0,
        KeyboardMouse,
        Gamepad
    } currentInputState = None;

    float GetAxis(string axis);

    void Update();
    void init();

}

#endif
