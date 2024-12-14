#include "ColliderObject.hpp" 
#include "globals.cpp" 

tmt::physics::ColliderObject::ColliderObject(ColliderInitInfo i, Object *parent)
{
    SetParent(parent);
    initInfo = i;
    var shape = ShapeFromInfo(i);
    if (i.s == Mesh)
    {
        var scale = convertVec3(GetGlobalScale());
        shape->setLocalScaling(scale);
    }

    shape->setUserIndex(-1);

    pId = collisionObjs.size();
    collisionObjs.push_back(shape);
}

