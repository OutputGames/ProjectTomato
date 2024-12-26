#include "input.hpp"
#include "globals.hpp"

using namespace tmt::input;

glm::vec2 Mouse::GetMousePosition()
{
    return mousep;
}

glm::vec2 Mouse::GetMouseDelta()
{
    return mousedelta;
}

// Function to convert mouse position to a ray direction
glm::vec3 ScreenToWorldRay(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix)
{
    // Convert mouse position to normalized device coordinates (NDC)
    float x = 1.0f - (2.0f * mouseX) / static_cast<float>(screenWidth);
    float y = 1.0f - (2.0f * mouseY) / static_cast<float>(screenHeight);
    float z = 1.0f;


    glm::vec3 rayNDC(x, y, z);

    // Convert NDC to clip coordinates
    glm::vec4 rayClip(rayNDC, 1.0f);

    // Convert clip coordinates to eye coordinates
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Convert eye coordinates to world coordinates
    auto rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayEye);
    rayWorld = normalize(rayWorld);

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


glm::vec3 Mouse::GetWorldMousePosition(render::Camera* camera)
{

    var dir = RaycastFromMouse(mousep.x, mousep.y, renderer->windowWidth, renderer->windowHeight, camera->GetView_m4(),
                               camera->GetProjection_m4());

    //debug::Gizmos::DrawSphere(camera->position + dir, 0.1f);

    var pos = camera->position;

    var ray = physics::Ray{pos, dir, 1000};

    var hit = ray.Cast();

    if (hit)
    {
        return hit->point;
    }

    return glm::vec3{0};
}

Mouse::MouseButtonState Mouse::GetMouseButton(MouseButton i)
{
    int state = glfwGetMouseButton(renderer->window, i);

    if (state == Press && (mstates[i] == Press || mstates[i] == Hold))
    {
        state = Hold;
    }

    mstates[i] = state;

    return static_cast<MouseButtonState>(state);
}

Keyboard::KeyState Keyboard::GetKey(int key)
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

Gamepad::PadState Gamepad::GetButton(int button)
{

}

float tmt::input::GetAxis(string axis)
{
    var a = 0.0f;

    if (axis == "Horizontal")
    {
        if (currentInputState == KeyboardMouse)
        {

            var l =
                (Keyboard::GetKey(GLFW_KEY_LEFT) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) == Keyboard::Hold);

            var r = (Keyboard::GetKey(GLFW_KEY_RIGHT) == Keyboard::Hold) ||
                (Keyboard::GetKey(GLFW_KEY_D) == Keyboard::Hold);


            if (l)
            {
                a = 1;
            }
            else if (r)
            {
                a = -1;
            }
        }
        else if (currentInputState == Gamepad)
        {

        }
    }
    else if (axis == "Vertical")
    {
        if (currentInputState == KeyboardMouse)
        {

            var u = (Keyboard::GetKey(GLFW_KEY_UP) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_W) ==
                Keyboard::Hold);

            var d = (Keyboard::GetKey(GLFW_KEY_DOWN) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) ==
                Keyboard::Hold);

            if (u)
            {
                a = 1;
            }
            else if (d)
            {
                a = -1;
            }
        }

    }

    return a;
}

void joystick_cb(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        currentInputState = Gamepad;
    }
    else if (event == GLFW_DISCONNECTED)
    {
        currentInputState = KeyboardMouse;
    }
}

void tmt::input::init()
{

}


void Update()
{

}
