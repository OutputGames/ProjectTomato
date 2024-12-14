#ifndef RAY_H
#define RAY_H

#include "utils.hpp" 
#include "tomato/Physics/RaycastHit.hpp"


namespace tmt::physics {

struct Ray
{
    glm::vec3 position, direction;
    float maxDistance = 10000;

    RaycastHit *Cast();
};

}

#endif