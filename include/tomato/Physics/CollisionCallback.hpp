#ifndef COLLISIONCALLBACK_H
#define COLLISIONCALLBACK_H

#include "utils.hpp" 
namespace tmt::physics {
 struct ColliderObject;
 };

namespace tmt::physics {
 struct PhysicsBody;
 };



namespace tmt::physics {

struct CollisionCallback : btCollisionWorld::ContactResultCallback
{
    PhysicsBody *body;
    ColliderObject *collider;

    // Overriding the callback method
    btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
                             const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) override;
};

}

#endif