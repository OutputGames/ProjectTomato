#ifndef COLLIDEROBJECT_H
#define COLLIDEROBJECT_H

#include "utils.hpp" 
#include "tomato/Obj/Object.hpp"
#include "tomato/Physics/ColliderInitInfo.hpp"
#include "tomato/Physics/CollisionCallback.hpp"


namespace tmt::physics {

struct ColliderObject : obj::Object
{
    ColliderInitInfo initInfo;
    ColliderObject(ColliderInitInfo info, Object *parent = nullptr);

  private:
    friend struct PhysicsBody;
    friend CollisionCallback;

    u16 pId = -1;
};

}

#endif