#ifndef COLLISION_H
#define COLLISION_H

#include "utils.hpp" 
namespace tmt::physics {
 struct PhysicsBody;
 };
#include "tomato/Physics/CollisionBase.hpp"


namespace tmt::physics {

struct Collision : CollisionBase
{
    PhysicsBody *other;
};

}

#endif