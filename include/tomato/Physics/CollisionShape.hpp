#ifndef COLLISIONSHAPE_H
#define COLLISIONSHAPE_H

#include "utils.hpp" 



namespace tmt::physics {

enum CollisionShape
{
    Box,
    Sphere,
    Capsule,
    Mesh
};

}

#endif