#include "Camera.hpp" 
#include "globals.cpp" 

float *tmt::render::Camera::GetView()
{
    var Front = GetFront();
    var Up = GetUp();

    float view[16];

    mtxLookAt(view, bx::Vec3(position.x, position.y, position.z), math::convertVec3(position + Front),
              bx::Vec3(Up.x, Up.y, Up.z));

    return view;
}

float *tmt::render::Camera::GetProjection()
{
    float proj[16];

    bx::mtxProj(proj, (FOV), static_cast<float>(renderer->windowWidth) / static_cast<float>(renderer->windowHeight),
                0.01f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    return proj;
}

glm::vec3 tmt::render::Camera::GetFront()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    direction.y = sin(glm::radians(rotation.x));
    direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    glm::vec3 Front = normalize(direction);

    return Front;
}

glm::vec3 tmt::render::Camera::GetUp()
{
    var Front = GetFront();

    glm::vec3 Right = normalize(cross(Front, glm::vec3{0, 1, 0}));
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower
    // movement.
    glm::vec3 Up = normalize(cross(Right, Front));

    return Up;
}

tmt::render::Camera *tmt::render::Camera::GetMainCamera()
{
    return mainCamera;
}

tmt::render::Camera::Camera()
{
    mainCamera = this;
}

