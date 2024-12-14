#include "Mouse.hpp" 
#include "globals.cpp" 

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

