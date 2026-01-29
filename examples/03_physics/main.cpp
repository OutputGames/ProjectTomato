/**
 * @file main.cpp
 * @brief Physics Demonstration Example
 * 
 * This example demonstrates:
 * - Physics simulation with Bullet3
 * - Creating physics bodies (dynamic and static)
 * - Collision detection
 * - Gravity and forces
 * - Spawning objects dynamically
 */

#include "tomato/tomato.hpp"
#include "tomato/globals.hpp"

using namespace tmt;

// Helper function to create a physics cube
obj::MeshObject* CreatePhysicsCube(glm::vec3 position, glm::vec3 size = glm::vec3(1.0f), float mass = 1.0f)
{
    var cube = obj::MeshObject::FromPrimitive(prim::Cube);
    if (!cube) return nullptr;

    cube->position = position;
    cube->scale = size;

    // Set random color
    if (cube->material)
    {
        auto colorUniform = cube->material->GetUniform("u_color");
        if (colorUniform)
        {
            colorUniform->v4 = glm::vec4(
                randomFloat(0.3f, 1.0f),
                randomFloat(0.3f, 1.0f),
                randomFloat(0.3f, 1.0f),
                1.0f
            );
        }
    }

    // Add physics body
    physics::ColliderInitInfo colliderInfo;
    colliderInfo.s = physics::Box;
    colliderInfo.bounds = size;

    var physicsBody = new physics::PhysicsBody(colliderInfo, mass);
    physicsBody->SetParent(cube);
    physicsBody->position = position;

    return cube;
}

int main(int argc, const char** argv)
{
    srand(static_cast<unsigned>(std::time(nullptr)));

    int width = 1280;
    int height = 720;
    
    var app = new engine::Application("Tomato Engine - Physics Demo", width, height, false);

    // Camera setup
    var cam = obj::CameraObject::GetMainCamera();
    if (cam)
    {
        cam->position = glm::vec3(15, 10, 15);
        cam->LookAt(glm::vec3(0, 0, 0));
    }

    // Create static ground plane
    var ground = obj::MeshObject::FromPrimitive(prim::Cube);
    if (ground)
    {
        ground->position = glm::vec3(0, -1, 0);
        ground->scale = glm::vec3(20, 0.5f, 20);
        
        if (ground->material)
        {
            auto colorUniform = ground->material->GetUniform("u_color");
            if (colorUniform)
            {
                colorUniform->v4 = glm::vec4(0.3f, 0.5f, 0.3f, 1.0f);
            }
        }

        // Add static physics body (mass = 0 means static)
        physics::ColliderInitInfo groundCollider;
        groundCollider.s = physics::Box;
        groundCollider.bounds = ground->scale;

        var groundPhysics = new physics::PhysicsBody(groundCollider, 0.0f);  // Mass 0 = static
        groundPhysics->SetParent(ground);
        groundPhysics->position = ground->position;
    }

    // Create some initial physics cubes
    CreatePhysicsCube(glm::vec3(-2, 5, 0), glm::vec3(1.0f), 1.0f);
    CreatePhysicsCube(glm::vec3(0, 8, 0), glm::vec3(1.0f), 1.0f);
    CreatePhysicsCube(glm::vec3(2, 11, 0), glm::vec3(1.0f), 1.0f);

    float spawnTimer = 0.0f;
    float spawnInterval = 2.0f;  // Spawn a new cube every 2 seconds
    int cubeCount = 3;
    const int maxCubes = 20;

    printf("\n=== PHYSICS DEMO ===\n");
    printf("Watch cubes fall and interact with physics!\n");
    printf("SPACE: Spawn a new cube\n");
    printf("R: Reset scene\n");
    printf("ESC: Exit\n\n");

    // Main game loop
    while (!glfwWindowShouldClose(renderer->window))
    {
        glfwPollEvents();

        float deltaTime = time::getDeltaTime();
        spawnTimer += deltaTime;

        // Auto-spawn cubes periodically
        if (spawnTimer >= spawnInterval && cubeCount < maxCubes)
        {
            // Random position above ground
            float x = randomFloat(-5.0f, 5.0f);
            float z = randomFloat(-5.0f, 5.0f);
            float y = randomFloat(10.0f, 15.0f);
            
            CreatePhysicsCube(glm::vec3(x, y, z), glm::vec3(1.0f), 1.0f);
            cubeCount++;
            spawnTimer = 0.0f;
            
            printf("Spawned cube %d at (%.1f, %.1f, %.1f)\n", cubeCount, x, y, z);
        }

        // Manual spawn with Space
        if (input::Keyboard::GetKey(GLFW_KEY_SPACE) == input::Keyboard::Press)
        {
            if (cubeCount < maxCubes)
            {
                float x = randomFloat(-5.0f, 5.0f);
                float z = randomFloat(-5.0f, 5.0f);
                CreatePhysicsCube(glm::vec3(x, 15.0f, z), glm::vec3(1.0f), 1.0f);
                cubeCount++;
                printf("Manually spawned cube %d\n", cubeCount);
            }
            else
            {
                printf("Maximum cubes reached (%d)\n", maxCubes);
            }
        }

        // Reset scene with R
        if (input::Keyboard::GetKey(GLFW_KEY_R) == input::Keyboard::Press)
        {
            printf("Resetting scene...\n");
            
            // Note: In a production app, you'd properly clean up all objects
            // For this demo, just note that you'd need to destroy physics bodies
            // and recreate the scene
            
            printf("Scene reset! (Note: Full reset not implemented in this demo)\n");
        }

        // Exit with ESC
        if (input::Keyboard::GetKey(GLFW_KEY_ESCAPE) == input::Keyboard::Press)
        {
            glfwSetWindowShouldClose(renderer->window, true);
        }

        // Update engine (includes physics simulation)
        engine::update();
    }

    engine::shutdown();

    return 0;
}
