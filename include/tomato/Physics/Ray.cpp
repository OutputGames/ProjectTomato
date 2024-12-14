#include "Ray.hpp" 
#include "globals.cpp" 

tmt::physics::RaycastHit *tmt::physics::Ray::Cast()
{
    auto start = convertVec3(position);
    auto end = convertVec3(position + (direction * maxDistance));
    btCollisionWorld::ClosestRayResultCallback callback(start, end);

    dynamicsWorld->rayTest(start, end, callback);

    if (callback.hasHit())
    {
        var point = callback.m_hitPointWorld;
        var nrm = callback.m_hitNormalWorld;

        var obj = callback.m_collisionObject;

        auto hit = new RaycastHit;

        hit->point = convertVec3(point);
        hit->normal = convertVec3(nrm);

        PhysicsBody *goA = nullptr;

        var goai = obj->getCollisionShape()->getUserIndex();

        if (goai != -1)
        {
            goA = bodies[goai];
        }

        hit->hit = goA;

        return hit;
    }
    return nullptr;
}

