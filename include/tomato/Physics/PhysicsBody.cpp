#include "PhysicsBody.hpp" 
#include "globals.cpp" 

tmt::physics::PhysicsBody::PhysicsBody(ColliderObject *collisionObj, float mass) : Object()
{
    if (!collisionObj->parent)
    {
        collisionObj->SetParent(this);
    }

    this->mass = mass;
    cPID = collisionObj->pId;

    collisionObjs[cPID]->setUserIndex(bodies.size());

    bodies.push_back(this);
}

void tmt::physics::PhysicsBody::Update()
{
    if (!parent)
        transRelation = Self;

    if (!doneFirstPhysicsUpdate)
    {
        pId = physicalBodies.size();

        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            collisionObjs[cPID]->calculateLocalInertia(mass, localInertia);

        btTransform startTransform;
        startTransform.setIdentity();

        if (transRelation == Self)
        {
            startTransform.setOrigin(convertVec3(position));
            startTransform.setRotation(convertQuat(rotation));
        }
        else
        {
            startTransform.setOrigin(convertVec3(parent->position));
            startTransform.setRotation(convertQuat(parent->rotation));
        }

        auto myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionObjs[cPID], localInertia);

        var rigidBody = new btRigidBody(rbInfo);

        rigidBody->setActivationState(DISABLE_DEACTIVATION);

        dynamicsWorld->addRigidBody(rigidBody);

        physicalBodies.push_back(rigidBody);
    }
    else
    {
        // var callback = *callback_;

        // dynamicsWorld->contactTest(physicalBodies[pId].object, callback);
    }

    var pBody = physicalBodies[pId];
    pBody->setUserPointer(this);
    // CONSTRAINTS !!! >x<

    if (pBody && pBody->getMotionState())
    {
        var motionState = pBody->getMotionState();
        btTransform transform;
        motionState->getWorldTransform(transform);
        // transform = pBody->getWorldTransform();

        if (transRelation == Self)
        {
            var p = position;

            position = convertVec3(transform.getOrigin());

            // transform.setOrigin(convertVec3(p));
        }
        else
        {
            var p = parent->position;
            var r = parent->rotation;

            parent->position = convertVec3(transform.getOrigin());

            parent->rotation = convertQuatEuler(transform.getRotation());

            transform.setIdentity();

            transform.setOrigin(convertVec3(p));
            transform.setRotation(convertQuat(r));

            motionState->setWorldTransform(transform);
        }
    }

    //

    // pBody->activate();
}

void tmt::physics::PhysicsBody::SetVelocity(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->setLinearVelocity(convertVec3(v));
}

glm::vec3 tmt::physics::PhysicsBody::GetVelocity()
{
    var pBody = physicalBodies[pId];

    return convertVec3(pBody->getLinearVelocity());
}

void tmt::physics::PhysicsBody::AddImpulse(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->applyCentralImpulse(convertVec3(v));
}

void tmt::physics::PhysicsBody::AddForce(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->applyCentralForce(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetAngular(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->setAngularVelocity(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetLinearFactor(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->setLinearFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetAngularFactor(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->setAngularFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetDamping(float linear, float angular)
{
    var pBody = physicalBodies[pId];

    pBody->setDamping(linear, angular);
}

void tmt::physics::PhysicsBody::AddCollisionEvent(std::function<void(Collision)> func)
{
    collisionEvents.push_back(func);
}

void tmt::physics::PhysicsBody::AddParticleCollisionEvent(std::function<void(ParticleCollision)> func)
{
    particleCollisionEvents.push_back(func);
}

glm::vec3 tmt::physics::PhysicsBody::GetBasisColumn(float v)
{
    var pBody = physicalBodies[pId];

    var basis = pBody->getWorldTransform().getBasis();

    var vector = basis.getColumn(v);

    return convertVec3(vector);
}

glm::vec3 tmt::physics::PhysicsBody::GetBasisRow(float v)
{
    var pBody = physicalBodies[pId];

    var basis = pBody->getWorldTransform().getBasis();

    var vector = basis.getRow(v);

    return convertVec3(vector);
}

void tmt::physics::PhysicsBody::OnCollision(Collision c)
{
    for (auto collision_event : collisionEvents)
    {
        collision_event(c);
    }
}

void tmt::physics::PhysicsBody::OnParticleCollision(ParticleCollision c)
{
    for (auto collision_event : particleCollisionEvents)
    {
        collision_event(c);
    }
}

