#ifndef RAYCASTHIT_H
#define RAYCASTHIT_H

#include "utils.hpp" 
#include "tomato/Physics/PhysicsBody.hpp"


namespace tmt::physics {

struct RaycastHit
{
    glm::vec3 point, normal;
    PhysicsBody *hit;
};

}

#endif