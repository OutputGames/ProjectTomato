#include "input.hpp" 
#include "globals.hpp" 

glm::vec2 tmt::input::Mouse::GetMousePosition()
{
    return mousep;
}

glm::vec2 tmt::input::Mouse::GetMouseDelta()
{
    return mousedelta;
}

tmt::input::Mouse::MouseButtonState tmt::input::Mouse::GetMouseButton(int i)
{
    int state = glfwGetMouseButton(renderer->window, i);

    if (state == Press && (mstates[i] == Press || mstates[i] == Hold))
    {
        state = Hold;
    }

    mstates[i] = state;

    return static_cast<MouseButtonState>(state);
}

tmt::input::Keyboard::KeyState tmt::input::Keyboard::GetKey(int key)
{
    int state = glfwGetKey(renderer->window, key);

    if (kstates.size() <= key)
    {
        kstates.resize(key + 1, Release);
        kstates[key] = state;
    }
    else
    {
        if (state == Press && (kstates[key] == Press || kstates[key] == Hold))
        {
            state = Hold;
        }
        kstates[key] = state;
    }

    return static_cast<KeyState>(state);
}
