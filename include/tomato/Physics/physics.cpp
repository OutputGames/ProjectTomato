#include "physics.hpp"
#include "globals.hpp"

using namespace tmt::physics;

std::vector<OBB*> obbs;

btScalar CollisionCallback::addSingleResult(btManifoldPoint& cp,
                                            const btCollisionObjectWrapper* colObj0Wrap, int partId0,
                                            int index0, const btCollisionObjectWrapper* colObj1Wrap,
                                            int partId1, int index1)
{
    btCollisionObjectWrapper *thisObj, *other;
    int thisIdx, otherIdx;
    int thisPart, otherPart;
    btVector3 thisPos, otherPos;
    btVector3 thisNormal, otherNormal;

    if (colObj0Wrap->getCollisionShape() == collisionObjs[collider->pId])
    {
        thisObj = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
        other = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);

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
        thisObj = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
        other = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);

        thisIdx = index1;
        otherIdx = index0;

        thisPart = partId1;
        otherPart = partId0;

        otherPos = cp.getPositionWorldOnA();
        otherNormal = -cp.m_normalWorldOnB;

        thisPos = cp.getPositionWorldOnB();
        thisPos = cp.m_normalWorldOnB;
    }

    var optr = other->getCollisionShape()->getUserPointer();
    var oidx = other->getCollisionShape()->getUserIndex();

    var particle = static_cast<particle::Particle*>(optr);
    PhysicsBody* obody = nullptr;

    if (oidx > -1)
        obody = bodies[oidx];

    if (particle)
    {
        ParticleCollision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;
        col.other = particle;

        body->OnParticleCollision(col);
    }
    else if (obody)
    {
        Collision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;

        col.other = obody;

        body->OnCollision(col);
    }

    return 0;
}

btScalar ParticleCollisionCallback::addSingleResult(btManifoldPoint& cp,
                                                    const btCollisionObjectWrapper* colObj0Wrap, int partId0,
                                                    int index0, const btCollisionObjectWrapper* colObj1Wrap,
                                                    int partId1, int index1)
{
    btCollisionObjectWrapper *thisObj, *other;
    int thisIdx, otherIdx;
    int thisPart, otherPart;
    btVector3 thisPos, otherPos;
    btVector3 thisNormal, otherNormal;

    if (colObj0Wrap->getCollisionShape() == collisionObjs[particle->cPID])
    {
        thisObj = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
        other = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);

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
        thisObj = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
        other = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);

        thisIdx = index1;
        otherIdx = index0;

        thisPart = partId1;
        otherPart = partId0;

        otherPos = cp.getPositionWorldOnA();
        otherNormal = -cp.m_normalWorldOnB;

        thisPos = cp.getPositionWorldOnB();
        thisPos = cp.m_normalWorldOnB;
    }

    var optr = other->getCollisionShape()->getUserPointer();
    var oidx = other->getCollisionShape()->getUserIndex();

    var particle = static_cast<particle::Particle*>(optr);
    PhysicsBody* obody = nullptr;

    if (oidx > -1)
        obody = bodies[oidx];

    if (particle)
    {
        ParticleCollision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;
        col.other = particle;

        particle->OnParticleCollision(col);
    }
    else if (obody)
    {
        Collision col{};

        col.contactPoint = convertVec3(thisPos);
        col.normal = convertVec3(thisNormal);
        col.faceId = thisIdx;

        col.other = obody;

        particle->OnCollision(col);
    }

    return 0;
}

btCollisionDispatcher* dispatcher;
btBroadphaseInterface* overlappingPairCache;
btSequentialImpulseConstraintSolver* solver;

PhysicalWorld::PhysicalWorld()
{
    auto configuration = new btDefaultCollisionConfiguration();

    dispatcher = new btCollisionDispatcher(configuration);

    overlappingPairCache = new btDbvtBroadphase();

    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);

    dynamicsWorld->setGravity(btVector3(0, -9.81, 0));

    SetLayerMask(0, {0, 1, 2, 3, 4, 5});
}

PhysicalWorld::~PhysicalWorld()
{
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;

    dynamicsWorld = nullptr;

    doneFirstPhysicsUpdate = true;
}

void ResolveCollision(OBB* a, OBB* b, const glm::vec3& mtv)
{
    if (a->isStatic && b->isStatic)
        return; // Both objects are static

    glm::vec3 mtvDir = normalize(mtv);
    float mtvMagnitude = glm::length(mtv);

    if (mtvMagnitude <= 0)
        return;

    // Separate objects based on their static status
    if (a->isStatic)
    {
        b->center += mtvDir * mtvMagnitude;
    }
    else if (b->isStatic)
    {
        a->center -= mtvDir * mtvMagnitude;
    }
    else
    {
        // Both are dynamic: split the translation
        a->center -= mtvDir * (mtvMagnitude / 2.0f);
        b->center += mtvDir * (mtvMagnitude / 2.0f);
    }

    // Reflect velocities for dynamic objects (optional for more realistic physics)
    if (!a->isStatic)
        a->velocity -= 2.0f * glm::dot(a->velocity, mtvDir) * mtvDir;
    if (!b->isStatic)
        b->velocity -= 2.0f * glm::dot(b->velocity, mtvDir) * mtvDir;
}


void PhysicalWorld::Update()
{
    dynamicsWorld->stepSimulation(1.0 / 6.0f, 1);

    dynamicsWorld->performDiscreteCollisionDetection();
    int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < nManifolds; i++)
    {
        btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();
        contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());
        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            // std::cout << "Collision between shapes " << obA->getCollisionShape()
            //	<< " and " << obB->getCollisionShape() << std::endl;

            PhysicsBody* goA = nullptr;
            PhysicsBody* goB = nullptr;

            var goai = obA->getCollisionShape()->getUserIndex();
            var gobi = obB->getCollisionShape()->getUserIndex();

            if (goai != -1)
            {
                goA = bodies[goai];
            }
            if (gobi != -1)
            {
                goB = bodies[gobi];
            }

            auto poA = static_cast<particle::Particle*>(obA->getCollisionShape()->getUserPointer());
            auto poB = static_cast<particle::Particle*>(obB->getCollisionShape()->getUserPointer());

            int exo = 0;

            for (int i = 0; i < 4; ++i)
            {
                switch (i)
                {
                    case 0:
                        if (goA)
                            exo++;
                        break;
                    case 1:
                        if (goB)
                            exo++;
                        break;
                    case 2:
                        if (poA)
                            exo++;
                        break;
                    case 3:
                        if (poB)
                            exo++;
                        break;
                }

                if (exo >= 2)
                    break;
            }

            if (exo == 2)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);
                btVector3 ptA, ptB;
                ptA = pt.getPositionWorldOnB();
                ptB = pt.getPositionWorldOnA();

                // std::cout << "Collision between " << goA->name << " and "	<< goB->name << std::endl;
                btVector3 nrmB = pt.m_normalWorldOnB;
                btVector3 nrmA = -nrmB;

                // ptA = convertVec3(glm::lerp(convertVec3(ptA), convertVec3(ptB), 0.5f));
                // ptB = ptA;

                var lpta = (obA->getWorldTransform().inverse() * ptA);
                var lptb = (obB->getWorldTransform().inverse() * ptB);
                var faceIdA = pt.m_index1;
                var faceIdB = pt.m_index0;

                /*
                if (goA)
                {
                    var colObj = goA->GetObjectFromType<ColliderObject>();
                    if (colObj)
                    {
                        if (colObj->initInfo.s == Mesh)
                        {
                            var mesh = colObj->initInfo.mesh;

                            glm::vec3 pta = convertVec3(ptA) / colObj->GetGlobalScale();
                            // pta = glm::vec3(glm::vec4(pta, 1) * glm::inverse(colObj->GetTransform()));

                            for (size_t triIndex = 0; triIndex < mesh->indexCount; triIndex += 3)
                            {
                                glm::vec3 v0 = mesh->vertices[mesh->indices[triIndex + 0]].position;
                                glm::vec3 v1 = mesh->vertices[mesh->indices[triIndex + 1]].position;
                                glm::vec3 v2 = mesh->vertices[mesh->indices[triIndex + 2]].position;

                                if (pointInTriangle(pta, v0, v1, v2, colObj->GetGlobalScale()))
                                {
                                    faceIdB = triIndex / 3;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (goB)
                {
                    var colObj = goB->GetObjectFromType<ColliderObject>();
                    if (colObj)
                    {
                        if (colObj->initInfo.s == Mesh)
                        {
                            var mesh = colObj->initInfo.mesh;

                            var ptb = convertVec3(ptB);
                            ptb = glm::vec3(glm::vec4(ptb, 1) * glm::inverse(colObj->GetTransform()));

                            var scale = colObj->GetGlobalScale();

                            for (size_t triIndex = 0; triIndex < mesh->indexCount; triIndex += 3)
                            {
                                glm::vec3 v0 = mesh->vertices[mesh->indices[triIndex + 0]].position;
                                glm::vec3 v1 = mesh->vertices[mesh->indices[triIndex + 1]].position;
                                glm::vec3 v2 = mesh->vertices[mesh->indices[triIndex + 2]].position;
                                if (pointInTriangle(ptb, v0, v1, v2, colObj->GetGlobalScale()))
                                {
                                    faceIdA = triIndex / 3;
                                    break;
                                }
                            }
                        }
                    }
                }
                */

                std::function<CollisionBase(bool)> CreateCollisionBase = [this, lpta, ptA, nrmA, lptb, ptB, nrmB,
                        faceIdA, faceIdB](bool a) -> CollisionBase
                {
                    btVector3 lpt, pta, nrma;
                    int fid;
                    if (a)
                    {
                        lpt = lpta;
                        pta = ptA;
                        nrma = nrmA;
                        fid = faceIdA;
                    }
                    else
                    {
                        lpt = lptb;
                        pta = ptB;
                        nrma = nrmB;
                        fid = faceIdB;
                    }
                    return CollisionBase(convertVec3(pta), convertVec3(nrma), fid);
                };

                if (goA && goB)
                {
                    ApplyTransform(goA, obA->getWorldTransform());
                    ApplyTransform(goB, obB->getWorldTransform());
                    {
                        auto c = static_cast<Collision>(CreateCollisionBase(true));
                        c.other = goB;
                        goA->OnCollision(c);
                    }
                    {
                        auto c = static_cast<Collision>(CreateCollisionBase(true));
                        c.other = goA;
                        goB->OnCollision(c);
                    }
                }
                if (goA && poB)
                {
                    ApplyTransform(goA, obA->getWorldTransform());
                    poB->position = convertVec3(obB->getWorldTransform().getOrigin());
                    poB->rotation = convertQuatEuler(obB->getWorldTransform().getRotation());

                    {
                        auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
                        c.other = poB;
                        goA->OnParticleCollision(c);
                    }
                    {
                        auto c = static_cast<Collision>(CreateCollisionBase(true));
                        c.other = goA;
                        poB->OnCollision(c);
                    }
                }
                if (goB && poA)
                {
                    ApplyTransform(goB, obB->getWorldTransform());

                    poA->position = convertVec3(obA->getWorldTransform().getOrigin());
                    poA->rotation = convertQuatEuler(obA->getWorldTransform().getRotation());

                    {
                        auto c = static_cast<Collision>(CreateCollisionBase(true));
                        c.other = goB;
                        poA->OnCollision(c);
                    }
                    {
                        auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
                        c.other = poA;
                        goB->OnParticleCollision(c);
                    }
                }
                if (poA && poB)
                {
                    poA->position = convertVec3(obA->getWorldTransform().getOrigin());
                    poA->rotation = convertQuatEuler(obA->getWorldTransform().getRotation());
                    poB->position = convertVec3(obB->getWorldTransform().getOrigin());
                    poB->rotation = convertQuatEuler(obB->getWorldTransform().getRotation());

                    {
                        auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
                        c.other = poB;
                        poA->OnParticleCollision(c);
                    }
                    {
                        auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
                        c.other = poA;
                        poB->OnParticleCollision(c);
                    }
                }
            }
        }
    }

    /*
    for (auto body : bodies)
    {
        dynamicsWorld->contactTest(physicalBodies[body->pId], body->callback_);
    }

    for (auto managed_particle : managed_particles)
    {
        dynamicsWorld->contactTest(physicalBodies[managed_particle->pId], managed_particle->_callback);
    }
    */


    for (auto collider : obbs)
    {
        if (collider->isStatic)
            continue;

        glm::vec3 mtv;
        for (auto obb : obbs)
        {
            if (obb == collider)
                continue;

            if (collider->Check(obb, mtv))
            {
                ResolveCollision(collider, obb, mtv);
            }
        }
    }
    for (auto value : obbs)
    {
        value->center += value->velocity * (1.0f / 60.0f);
    }

    doneFirstPhysicsUpdate = true;
}

void PhysicalWorld::RemoveBody(int pid, int cpid)
{
    dynamicsWorld->removeRigidBody(physicalBodies[pid]);

    physicalBodies.erase(physicalBodies.begin() + pid);
    collisionObjs.erase(collisionObjs.begin() + cpid);

    for (auto body : bodies)
    {
        if (body->pId > pid)
            body->pId--;
        if (body->cPID > cpid)
            body->cPID--;
    }

    for (auto managed_particle : managed_particles)
    {
        if (managed_particle->pId > pid)
        {
            managed_particle->pId--;
        }
        if (managed_particle->cPID > cpid)
        {
            managed_particle->cPID--;
        }
    }
}


std::vector<PhysicsBody*> PhysicalWorld::GetGameObjectsCollidingWith(PhysicsBody* collider)
{
    {
        std::vector<PhysicsBody*> collisions;

        int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
        for (int i = 0; i < nManifolds; i++)
        {
            btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject* obA = contactManifold->getBody0();
            const btCollisionObject* obB = contactManifold->getBody1();

            auto goA = static_cast<PhysicsBody*>(obA->getCollisionShape()->getUserPointer());
            auto goB = static_cast<PhysicsBody*>(obB->getCollisionShape()->getUserPointer());
            //if (goA != nullptr && goB != nullptr)
            //std::cout << "Collision between " << goA->name << " and " << goB->name << std::endl;
            if (goA == collider && goB != nullptr &&
                std::find(collisions.begin(), collisions.end(), goB) == collisions.end())
                collisions.push_back(goB);
            if (goB == collider && goA != nullptr &&
                std::find(collisions.begin(), collisions.end(), goA) == collisions.end())
                collisions.push_back(goA);
        }

        return collisions;

        /*
        std::vector<GameObject*> collisions;
        btCollisionObjectArray coArray = dynamicsWorld->getCollisionObjectArray();
        int nCollisionObjects = coArray.size();
        for (int i = 0; i < nCollisionObjects; i++)
        {
            btCollisionObject* cObj = coArray.at(i);
            if (collider->checkCollideWith(cObj))
                collisions.push_back(static_cast<GameObject*>(cObj->getCollisionShape()->getUserPointer()));
        }
        return collisions;
        */
    }
}

void PhysicalWorld::SetLayerMask(int layer, std::vector<int> mask)
{
    if (layerMasks.size() <= layer)
    {
        layerMasks.resize(layer + 1);
    }

    int m = 0;

    for (int value : mask)
    {
        m |= (value);
    }

    layerMasks[layer] = m;

}

ColliderInitInfo ColliderInitInfo::ForBox(glm::vec3 bounds)
{

    var info = ColliderInitInfo();

    info.bounds = bounds;

    info.s = Box;

    return info;
}

ColliderInitInfo ColliderInitInfo::ForSphere(float radius)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.s = Sphere;

    return info;
}

ColliderInitInfo ColliderInitInfo::ForCapsule(float radius, float height)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.height = height;
    info.s = Capsule;

    return info;
}

ColliderInitInfo ColliderInitInfo::ForMesh(render::Mesh* mesh)
{
    var info = ColliderInitInfo();

    info.mesh = mesh;
    info.s = Mesh;

    return info;
}

ColliderObject::ColliderObject(ColliderInitInfo i, Object* parent, bool scaleByObject)
{
    this->scaleByObject = scaleByObject;
    SetParent(parent);

    Init(i);
}

ColliderObject::ColliderObject()
{

}

void ColliderObject::Init(ColliderInitInfo i)
{
    initInfo = i;
    var shape = ShapeFromInfo(i);
    if (i.s == Mesh || scaleByObject)
    {
        var scale = convertVec3(GetGlobalScale());
        shape->setLocalScaling(scale);
    }

    shape->setUserIndex(-1);

    pId = collisionObjs.size();
    collisionObjs.push_back(shape);
}

void PhysicsBody::Init(ColliderObject* collisionObj, float mass)
{
    if (!collisionObj->parent)
    {
        collisionObj->SetParent(this);
    }

    this->mass = mass;
    cPID = collisionObj->pId;

    collisionObjs[cPID]->setUserIndex(bodies.size());

    callback_ = CollisionCallback();
    callback_.body = this;
    callback_.collider = collisionObj;

    bodies.push_back(this);
}

PhysicsBody::PhysicsBody(ColliderObject* collisionObj, float mass) :
    Object()
{
    Init(collisionObj, mass);
}

PhysicsBody::PhysicsBody()
{

}

void PhysicsBody::SetForward(glm::vec3 v)
{
    Object::SetForward(v);

    if (pId >= physicalBodies.size())
        return;

    var body = physicalBodies[pId];

    if (body)
    {
        var t = body->getWorldTransform();

        t.setRotation(convertQuat(rotation));

        body->setWorldTransform(t);
    }
}

void PhysicsBody::Update()
{
    if (!parent)
        transRelation = Self;

    if ((!doneFirstPhysicsUpdate || pId == UINT16_MAX) && dynamicsWorld != nullptr)
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
        rigidBody->setUserIndex2(physicalBodies.size());

        var physicalWorld = obj::Scene::GetMainScene()->physicsWorld;

        // adding physics masks!

        dynamicsWorld->addRigidBody(rigidBody, layer, physicalWorld->layerMasks[layer]);

        physicalBodies.push_back(rigidBody);
    }
    else
    {
        // var callback = *callback_;

        // dynamicsWorld->contactTest(physicalBodies[pId].object, callback);
    }

    btRigidBody* pBody = nullptr;

    if (physicalBodies.size() > pId)
    {
        pBody = physicalBodies[pId];
        pBody->setUserPointer(this);
    }

    if (pBody && pBody->getMotionState())
    {
        var motionState = pBody->getMotionState();
        btTransform transform;
        motionState->getWorldTransform(transform);
        transform = pBody->getWorldTransform();

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

            var np = convertVec3(transform.getOrigin());
            var nr = convertQuat(transform.getRotation());

            parent->position = np;
            parent->rotation = nr;

            var diff = p - np;
            var qdiff = r - nr;
            float tolerance = 1;

            if (glm::length(diff) > tolerance)
            {
                transform.setOrigin(convertVec3(p));
            }
            if (glm::length(qdiff) > tolerance)
            {
                transform.setRotation(convertQuat(r));
            }

            pBody->setWorldTransform(transform);
        }
    }

    //

    // pBody->activate();

    Object::Update();
}

void PhysicsBody::SetVelocity(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setLinearVelocity(convertVec3(v));
}

glm::vec3 PhysicsBody::GetVelocity()
{
    if (pId >= physicalBodies.size())
        return glm::vec3{0};
    var pBody = physicalBodies[pId];

    return convertVec3(pBody->getLinearVelocity());
}

void PhysicsBody::SetPushVelocity(glm::vec3 v)
{
    var pBody = physicalBodies[pId];

    pBody->setPushVelocity(convertVec3(v));
}

void PhysicsBody::SetAngular(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setAngularVelocity(convertVec3(v));
}

void PhysicsBody::AddImpulse(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->applyCentralImpulse(convertVec3(v));
}

void PhysicsBody::AddForce(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->applyCentralForce(convertVec3(v));
}

void PhysicsBody::SetLinearFactor(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setLinearFactor(convertVec3(v));
}

void PhysicsBody::SetAngularFactor(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setAngularFactor(convertVec3(v));
}

void PhysicsBody::SetDamping(float linear, float angular)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setDamping(linear, angular);
}

void PhysicsBody::AddCollisionEvent(std::function<void(Collision)> func)

{

    collisionEvents.push_back(func);
}

void PhysicsBody::AddParticleCollisionEvent(std::function<void(ParticleCollision)> func)

{

    particleCollisionEvents.push_back(func);
}

glm::vec3 PhysicsBody::GetBasisColumn(float v)
{
    var pBody = physicalBodies[pId];

    var basis = pBody->getWorldTransform().getBasis();

    var vector = basis.getColumn(v);

    return convertVec3(vector);
}

glm::vec3 PhysicsBody::GetBasisRow(float v)
{
    var pBody = physicalBodies[pId];

    var basis = pBody->getWorldTransform().getBasis();

    var vector = basis.getRow(v);

    return convertVec3(vector);
}

void PhysicsBody::Reset()
{
    var pBody = physicalBodies[pId];

    pBody->setAngularVelocity({0, 0, 0});
    pBody->setLinearVelocity({0, 0, 0});
}

PhysicsBody::~PhysicsBody()
{
    mainScene->physicsWorld->RemoveBody(pId, cPID);
}

void PhysicsBody::OnCollision(Collision c)
{
    for (auto collision_event : collisionEvents)
    {
        collision_event(c);
    }
}

void PhysicsBody::OnParticleCollision(ParticleCollision c)
{
    for (auto collision_event : particleCollisionEvents)
    {
        collision_event(c);
    }
}

RaycastHit* Ray::Cast()
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

        PhysicsBody* goA = nullptr;

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

OBB::OBB()
{
    obbs.push_back(this);
}

// Check for overlap on a given axis
bool overlapOnAxis(const glm::vec3& posA, const glm::vec3& halfSizeA, const glm::mat3& rotA, const glm::vec3& posB,
                   const glm::vec3& halfSizeB, const glm::mat3& rotB, const glm::vec3& axis, float& depth)
{
    auto project = [](const glm::vec3& pos, const glm::vec3& halfSize, const glm::mat3& rot, const glm::vec3& axis)
    {
        glm::vec3 corners[8] = {pos + rot * glm::vec3(halfSize.x, halfSize.y, halfSize.z),
                                pos + rot * glm::vec3(-halfSize.x, halfSize.y, halfSize.z),
                                pos + rot * glm::vec3(halfSize.x, -halfSize.y, halfSize.z),
                                pos + rot * glm::vec3(-halfSize.x, -halfSize.y, halfSize.z),
                                pos + rot * glm::vec3(halfSize.x, halfSize.y, -halfSize.z),
                                pos + rot * glm::vec3(-halfSize.x, halfSize.y, -halfSize.z),
                                pos + rot * glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
                                pos + rot * glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z)};
        float min = glm::dot(axis, corners[0]);
        float max = min;
        for (const glm::vec3& corner : corners)
        {
            float projection = glm::dot(axis, corner);
            if (projection < min)
                min = projection;
            if (projection > max)
                max = projection;
        }
        return std::make_pair(min, max);
    };

    auto [aMin, aMax] = project(posA, halfSizeA, rotA, axis);
    auto [bMin, bMax] = project(posB, halfSizeB, rotB, axis);

    if (aMax < bMin || bMax < aMin)
    {
        return false; // No overlap
    }

    float overlap1 = aMax - bMin;
    float overlap2 = bMax - aMin;
    depth = std::min(overlap1, overlap2);
    return true; // Overlap found
}

// Get the axes to test for separation
std::vector<glm::vec3> getAxes(const glm::mat3& rotA, const glm::mat3& rotB)
{
    std::vector<glm::vec3> axes;

    for (int i = 0; i < 3; ++i)
    {
        axes.push_back(rotA[i]);
        axes.push_back(rotB[i]);
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            axes.push_back(cross(rotA[i], rotB[j]));
        }
    }

    return axes;
}

bool testOBBOBB(const glm::vec3& posA, const glm::vec3& halfSizeA, const glm::mat3& rotA, const glm::vec3& posB,
                const glm::vec3& halfSizeB, const glm::mat3& rotB, glm::vec3& mtv)
{
    std::vector<glm::vec3> axes = getAxes(rotA, rotB);
    float minOverlap = std::numeric_limits<float>::max();
    glm::vec3 smallestAxis;

    for (const glm::vec3& axis : axes)
    {
        float overlap;
        if (!overlapOnAxis(posA, halfSizeA, rotA, posB, halfSizeB, rotB, axis, overlap))
        {
            return false; // Separating axis found, no collision
        }
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    mtv = smallestAxis * minOverlap;
    return true; // Collision detected
}

bool OBB::Check(OBB* other, glm::vec3& mtv)
{
    // Axes of this and other OBB
    glm::vec3 axesA[3] = {axis[0], axis[1], axis[2]};
    glm::vec3 axesB[3] = {other->axis[0], other->axis[1], other->axis[2]};

    // Compute the rotation matrix expressing other in this OBB's local space
    glm::mat3 R, AbsR;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            R[i][j] = glm::dot(axesA[i], axesB[j]);
            AbsR[i][j] = std::abs(R[i][j]) + EPSILON; // Add a small epsilon to counter floating-point errors
        }
    }

    // Compute the translation vector
    glm::vec3 t = other->center - center;
    // Bring translation into this OBB's local space
    t = glm::vec3(glm::dot(t, axesA[0]), glm::dot(t, axesA[1]), glm::dot(t, axesA[2]));

    // Variables to track minimum overlap and axis
    float minOverlap = FLT_MAX;
    glm::vec3 collisionAxis;

    // Test the three axes of this OBB
    for (int i = 0; i < 3; i++)
    {
        float ra = halfSize[i];
        float rb = other->halfSize[0] * AbsR[i][0] + other->halfSize[1] * AbsR[i][1] + other->halfSize[2] * AbsR[i][2];
        float overlap = ra + rb - std::abs(t[i]);
        if (overlap < 0)
            return false; // Separating axis found
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            collisionAxis = axesA[i] * glm::sign(t[i]);
        }
    }

    // Test the three axes of the other OBB
    for (int i = 0; i < 3; i++)
    {
        float ra = halfSize[0] * AbsR[0][i] + halfSize[1] * AbsR[1][i] + halfSize[2] * AbsR[2][i];
        float rb = other->halfSize[i];
        float overlap = ra + rb - std::abs(glm::dot(t, axesB[i]));
        if (overlap < 0)
            return false; // Separating axis found
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            collisionAxis = axesB[i] * glm::sign(glm::dot(t, axesB[i]));
        }
    }

    // Test cross products of axes
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            glm::vec3 axis = cross(axesA[i], axesB[j]);
            if (glm::length(axis) < EPSILON)
                continue; // Skip near-zero axes

            float ra = halfSize[(i + 1) % 3] * AbsR[(i + 2) % 3][j] + halfSize[(i + 2) % 3] * AbsR[(i + 1) % 3][j];
            float rb = other->halfSize[(j + 1) % 3] * AbsR[i][(j + 2) % 3] +
                other->halfSize[(j + 2) % 3] * AbsR[i][(j + 1) % 3];
            float dist = std::abs(glm::dot(t, axis));
            float overlap = ra + rb - dist;

            if (overlap < 0)
                return false; // Separating axis found
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                collisionAxis = normalize(axis) * glm::sign(glm::dot(t, axis));
            }
        }
    }

    // Store the Minimum Translation Vector (MTV)
    mtv = collisionAxis * minOverlap;
    return true; // Collision detected
}


OBB* OBB::FromBox(glm::vec3 position, glm::vec3 size, glm::quat rotation)
{
    var box = new OBB;

    box->center = position;
    box->halfSize = size / 2.0f;
    box->axis = mat3_cast(rotation);

    return box;
}
