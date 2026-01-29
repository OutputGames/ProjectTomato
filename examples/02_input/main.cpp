/**
 * @file main.cpp
 * @brief Input Demonstration Example
 * 
 * This example demonstrates:
 * - Keyboard input (WASD for movement, Space for jump)
 * - Mouse input (look around, click to change cube color)
 * - Gamepad input (if connected)
 * - Camera control
 * - Interactive object manipulation
 */

#include "tomato/tomato.hpp"
#include "tomato/globals.hpp"

using namespace tmt;

int main(int argc, const char** argv)
{
    srand(static_cast<unsigned>(std::time(nullptr)));

    int width = 1280;
    int height = 720;
    
    var app = new engine::Application("Tomato Engine - Input Demo", width, height, false);

    // Camera setup
    var cam = obj::CameraObject::GetMainCamera();
    if (cam)
    {
        cam->position = glm::vec3(0, 2, 5);
        cam->LookAt(glm::vec3(0, 0, 0));
    }

    // Create interactive cube
    var cube = obj::MeshObject::FromPrimitive(prim::Cube);
    glm::vec4 cubeColor = glm::vec4(0.3f, 0.7f, 0.9f, 1.0f);
    
    if (cube && cube->material)
    {
        auto colorUniform = cube->material->GetUniform("u_color");
        if (colorUniform)
        {
            colorUniform->v4 = cubeColor;
        }
    }

    // Create ground plane
    var ground = obj::MeshObject::FromPrimitive(prim::Cube);
    if (ground)
    {
        ground->position = glm::vec3(0, -1, 0);
        ground->scale = glm::vec3(10, 0.1f, 10);
        
        if (ground->material)
        {
            auto colorUniform = ground->material->GetUniform("u_color");
            if (colorUniform)
            {
                colorUniform->v4 = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
            }
        }
    }

    // Camera control variables
    float cameraDistance = 5.0f;
    float cameraAngleH = 0.0f;
    float cameraAngleV = 30.0f;
    float cameraSensitivity = 0.3f;

    printf("\n=== INPUT DEMO CONTROLS ===\n");
    printf("WASD/Arrow Keys: Move camera around\n");
    printf("Mouse Move: Look around (hold right mouse button)\n");
    printf("Left Click: Change cube color\n");
    printf("Scroll: Zoom in/out\n");
    printf("Space: Reset camera\n");
    printf("ESC: Exit\n\n");

    // Main game loop
    while (!glfwWindowShouldClose(renderer->window))
    {
        glfwPollEvents();

        // === Keyboard Input ===
        
        // Get horizontal and vertical input (-1 to 1)
        float horizontal = input::GetAxis("Horizontal");
        float vertical = input::GetAxis("Vertical");

        // Move cube with keyboard
        if (cube)
        {
            float moveSpeed = 2.0f * time::getDeltaTime();
            cube->position.x += horizontal * moveSpeed;
            cube->position.z += vertical * moveSpeed;
        }

        // Reset camera with Space
        if (input::Keyboard::GetKey(GLFW_KEY_SPACE) == input::Keyboard::Press)
        {
            cameraDistance = 5.0f;
            cameraAngleH = 0.0f;
            cameraAngleV = 30.0f;
            printf("Camera reset!\n");
        }

        // Exit with ESC
        if (input::Keyboard::GetKey(GLFW_KEY_ESCAPE) == input::Keyboard::Press)
        {
            glfwSetWindowShouldClose(renderer->window, true);
        }

        // === Mouse Input ===
        
        // Change cube color on left click
        if (input::Mouse::GetMouseButton(input::Mouse::Left) == input::Mouse::Press)
        {
            cubeColor = glm::vec4(
                randomFloat(0.0f, 1.0f),
                randomFloat(0.0f, 1.0f),
                randomFloat(0.0f, 1.0f),
                1.0f
            );
            
            if (cube && cube->material)
            {
                auto colorUniform = cube->material->GetUniform("u_color");
                if (colorUniform)
                {
                    colorUniform->v4 = cubeColor;
                }
            }
            
            printf("Cube color changed!\n");
        }

        // Camera rotation with right mouse button
        if (input::Mouse::GetMouseButton(input::Mouse::Right) == input::Mouse::Hold)
        {
            auto mouseDelta = input::Mouse::GetMouseDelta();
            cameraAngleH -= mouseDelta.x * cameraSensitivity;
            cameraAngleV += mouseDelta.y * cameraSensitivity;
            
            // Clamp vertical angle
            cameraAngleV = glm::clamp(cameraAngleV, -89.0f, 89.0f);
        }

        // Zoom with mouse scroll
        auto scroll = input::Mouse::GetMouseScroll();
        if (scroll.y != 0)
        {
            cameraDistance -= scroll.y * 0.5f;
            cameraDistance = glm::clamp(cameraDistance, 2.0f, 20.0f);
        }

        // === Camera Update ===
        
        if (cam && cube)
        {
            // Calculate camera position based on angles and distance
            float radH = glm::radians(cameraAngleH);
            float radV = glm::radians(cameraAngleV);
            
            glm::vec3 offset;
            offset.x = cos(radV) * sin(radH) * cameraDistance;
            offset.y = sin(radV) * cameraDistance;
            offset.z = cos(radV) * cos(radH) * cameraDistance;
            
            cam->position = cube->position + offset;
            cam->LookAt(cube->position);
        }

        // Update engine
        engine::update();
    }

    engine::shutdown();

    return 0;
}
