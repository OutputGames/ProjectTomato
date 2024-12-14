#ifndef CAMERA_H
#define CAMERA_H

#include "utils.hpp" 
namespace tmt::obj {
 struct CameraObject;
 };



namespace tmt::render {

struct Camera
{
    glm::vec3 position, rotation;
    float FOV = 90.0f;

    float *GetView();
    float *GetProjection();

    glm::vec3 GetFront();
    glm::vec3 GetUp();

    static Camera *GetMainCamera();

  private:
    friend obj::CameraObject;

    Camera();
};

}

#endif