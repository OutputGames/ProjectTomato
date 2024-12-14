#ifndef INPUT_H
#define INPUT_H

#include "utils.hpp" 





namespace tmt::input {

struct Mouse;
struct Keyboard;

struct Mouse
{
    static glm::vec2 GetMousePosition();
    static glm::vec2 GetMouseDelta();

    enum MouseButtonState
    {
        Release = GLFW_RELEASE,
        Press = GLFW_PRESS,
        Hold,
    };

    static MouseButtonState GetMouseButton(int i);
};

struct Keyboard
{
    enum KeyState
    {
        Release = GLFW_RELEASE,
        Press = GLFW_PRESS,
        Hold,
    };

    static KeyState GetKey(int key);
};
;

}

#endif