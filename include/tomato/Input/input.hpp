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
            Left   = GLFW_MOUSE_BUTTON_LEFT,
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
        static float GetAxis(int axis);
    };

    enum InputState
    {
        None = 0,
        KeyboardMouse,
        Gamepad
    };

    InputState GetInputState();

    float GetAxis(string axis);
    glm::vec2 GetAxis2(string axis);

    string GetGamepadName();

    void Update();
    void init();

}

#endif
