#ifndef PHYSICALWORLD_H
#define PHYSICALWORLD_H

#include "utils.hpp" 
#include "tomato/Physics/PhysicsBody.hpp"
#include "tomato/Physics/PhysicsBody.hpp"


namespace tmt::physics {

struct PhysicalWorld
{
    PhysicalWorld();

    void Update();

    void RemoveBody(int pid, int cpid);

    std::vector<PhysicsBody *> GetGameObjectsCollidingWith(PhysicsBody *collider);
};

}

#endif