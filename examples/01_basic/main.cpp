/**
 * @file main.cpp
 * @brief Basic Tomato Engine Example
 * 
 * This example demonstrates:
 * - Engine initialization
 * - Creating a window
 * - Basic rendering loop
 * - Rendering a spinning cube
 * - Clean shutdown
 */

#include "tomato/tomato.hpp"
#include "tomato/globals.hpp"

using namespace tmt;

int main(int argc, const char** argv)
{
    // Seed random number generator
    srand(static_cast<unsigned>(std::time(nullptr)));

    // Window configuration
    int width = 1280;
    int height = 720;
    
    // Create the application
    // Parameters: window title, width, height, is2D (false = 3D)
    var app = new engine::Application("Tomato Engine - Basic Example", width, height, false);

    // Get the main camera
    var cam = obj::CameraObject::GetMainCamera();
    if (cam)
    {
        // Position camera to see the scene
        cam->position = glm::vec3(5, 5, 5);
        cam->LookAt(glm::vec3(0, 0, 0));
    }

    // Create a cube mesh object from primitive
    var cube = obj::MeshObject::FromPrimitive(prim::Cube);
    if (cube)
    {
        // Set cube color (RGBA)
        if (cube->material)
        {
            auto colorUniform = cube->material->GetUniform("u_color");
            if (colorUniform)
            {
                colorUniform->v4 = glm::vec4(0.8f, 0.3f, 0.5f, 1.0f);
            }
        }
    }

    // Main game loop
    while (!glfwWindowShouldClose(renderer->window))
    {
        // Poll for events (keyboard, mouse, etc.)
        glfwPollEvents();

        // Rotate the cube
        if (cube)
        {
            float t = time::getTime() * 0.01f;
            cube->rotation.y = t * 50.0f;  // Rotate around Y axis
            cube->rotation.x = t * 30.0f;  // Rotate around X axis
        }

        // Update all engine systems
        engine::update();
    }

    // Clean shutdown
    engine::shutdown();

    return 0;
}
