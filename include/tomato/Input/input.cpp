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

glm::vec3 unprojectMouseToWorld(int mouseX, int mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                                const glm::mat4& projMatrix)
{
    // Convert mouse coordinates to normalized device coordinates (NDC)
    float ndcX = (2.0f * flt mouseX) / flt screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * flt mouseY) / flt screenHeight; // Invert Y axis for NDC
    float ndcZ = 1.0f;

    glm::vec3 ndc{ndcX, ndcY, ndcZ};

    // Create a point in NDC space
    glm::vec4 clipCoords = glm::vec4(ndcX, ndcY, -1.0f, 1.0f); // Use -1.0 for near clipping plane

    glm::vec4 ray_eye = glm::inverse(projMatrix) * clipCoords;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1, 0.0);

    glm::vec3 ray_wor = (glm::inverse(viewMatrix) * ray_eye);

    ray_wor = glm::normalize(ray_wor);

    return ray_wor;
}

glm::vec3 tmt::input::Mouse::GetWorldMousePosition(render::Camera* camera)
{
    return unprojectMouseToWorld(mousep.x, mousep.y, renderer->windowWidth, renderer->windowHeight, camera->GetView_m4(),
                                 camera->GetProjection_m4());
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

float tmt::input::GetAxis(string axis)
{
    var a = 0.0f;

    if (axis == "Horizontal")
    {

        var l = (Keyboard::GetKey(GLFW_KEY_LEFT) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) == Keyboard::Hold);

        var r =
            (Keyboard::GetKey(GLFW_KEY_RIGHT) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_D) == Keyboard::Hold);

        if (l)
        {
            a = 1;
        }
        else if (r)
        {
            a = -1;
        }
    }
    else if (axis == "Vertical")
    {

        var u = (Keyboard::GetKey(GLFW_KEY_UP) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_W) == Keyboard::Hold);

        var d = (Keyboard::GetKey(GLFW_KEY_DOWN) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) == Keyboard::Hold);

        if (u)
        {
            a = 1;
        }
        else if (d)
        {
            a = -1;
        }
    }

    return a;
}

