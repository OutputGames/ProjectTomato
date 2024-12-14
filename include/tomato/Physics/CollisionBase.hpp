#ifndef COLLISIONBASE_H
#define COLLISIONBASE_H

#include "utils.hpp" 



namespace tmt::physics {

struct CollisionBase
{
    glm::vec3 contactPoint;
    glm::vec3 normal;
    int faceId;
};

}

#endif