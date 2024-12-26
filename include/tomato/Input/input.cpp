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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function to convert mouse position to a ray direction
glm::vec3 ScreenToWorldRay(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix)
{
    // Convert mouse position to normalized device coordinates (NDC)
    float x = (2.0f * mouseX) / flt screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / flt screenHeight;
    float z = 1.0f;
    glm::vec3 rayNDC(x, y, z);

    // Convert NDC to clip coordinates
    glm::vec4 rayClip(rayNDC, 1.0f);

    // Convert clip coordinates to eye coordinates
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Convert eye coordinates to world coordinates
    glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    return rayWorld;
}

// Example usage
glm::vec3 RaycastFromMouse(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                      const glm::mat4& projectionMatrix)
{
    glm::vec3 rayDirection = ScreenToWorldRay(mouseX, mouseY, screenWidth, screenHeight, viewMatrix, projectionMatrix);


    // Now you can use rayDirection to perform raycasting in your scene
    // For example, intersecting with objects in the scene

    return rayDirection;
}


glm::vec3 tmt::input::Mouse::GetWorldMousePosition(render::Camera* camera)
{
    /*
    var dir = RaycastFromMouse(mousep.x, mousep.y, renderer->windowWidth, renderer->windowHeight, camera->GetView_m4(),
                     camera->GetProjection_m4());

    var pos = camera->position;

    var ray = physics::Ray{pos, dir, 100};

    var hit = ray.Cast();

    if (hit)
    {
        return hit->point;
    }
    */

    if (mousep.x < 0 || mousep.x > renderer->windowWidth || mousep.y < 0 || mousep.x > renderer->windowHeight)
    {
        return glm::vec3{0};
    } 

    float x = 2.0f * mousep.x / renderer->windowWidth - 1;
    float y = 2.0f * mousep.y / renderer->windowHeight - 1;

    glm::vec4 screenPos = glm::vec4(x, -y, -1.0f, 1.0f);

    glm::mat4 projView = camera->GetProjection_m4() * camera->GetView_m4();
    glm::mat4 viewProjInv = glm::inverse(projView);

    glm::vec4 worldPos = viewProjInv * screenPos;

    return glm::vec3(worldPos);
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

