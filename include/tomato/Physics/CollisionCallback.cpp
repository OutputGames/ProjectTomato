#include "CollisionCallback.hpp" 
#include "globals.cpp" 

btScalar tmt::physics::CollisionCallback::addSingleResult(btManifoldPoint &cp,
                                                          const btCollisionObjectWrapper *colObj0Wrap, int partId0,
                                                          int index0, const btCollisionObjectWrapper *colObj1Wrap,
                                                          int partId1, int index1)
{
    btCollisionObjectWrapper *thisObj, *other;
    int thisIdx, otherIdx;
    int thisPart, otherPart;
    btVector3 thisPos, otherPos;
    btVector3 thisNormal, otherNormal;

    if (colObj0Wrap->getCollisionShape() == collisionObjs[collider->pId])
    {
        thisObj = const_cast<btCollisionObjectWrapper *>(colObj0Wrap);
        other = const_cast<btCollisionObjectWrapper *>(colObj1Wrap);

        thisIdx = index0;
        otherIdx = index1;

        thisPart = partId0;
        otherPart = partId1;

        thisPos = cp.getPositionWorldOnA();
        thisNormal = -cp.m_normalWorldOnB;

        otherPos = cp.getPositionWorldOnB();
        otherNormal = cp.m_normalWorldOnB;
    }
    else
    {
        thisObj = const_cast<btCollisionObjectWrapper *>(colObj1Wrap);
        other = const_cast<btCollisionObjectWrapper *>(colObj0Wrap);

        thisIdx = index1;
        otherIdx = index0;

        thisPart = partId1;
        otherPart = partId0;

        otherPos = cp.getPositionWorldOnA();
        otherNormal = -cp.m_normalWorldOnB;

        thisPos = cp.getPositionWorldOnB();
        thisPos = cp.m_normalWorldOnB;
    }

    var particle = static_cast<particle::Particle *>(other->getCollisionShape()->getUserPointer());

    if (particle)
    {
        ParticleCollision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;
        col.other = particle;

        body->OnParticleCollision(col);
    }
    else
    {
        Collision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;
        col.other = bodies[other->getCollisionShape()->getUserIndex()];

        body->OnCollision(col);
    }

    return 0;
}

