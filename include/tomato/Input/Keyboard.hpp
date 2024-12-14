#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utils.hpp" 



namespace tmt::input {

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

}

#endif