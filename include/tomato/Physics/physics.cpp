#include "physics.hpp" 
#include "globals.hpp" 

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

btScalar tmt::physics::ParticleCollisionCallback::addSingleResult(btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap,
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

tmt::physics::PhysicalWorld::PhysicalWorld()
{
    auto configuration = new btDefaultCollisionConfiguration();

    auto dispatcher = new btCollisionDispatcher(configuration);

    btBroadphaseInterface *overlappingPairCache = new btDbvtBroadphase();

    auto solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);

    dynamicsWorld->setGravity(btVector3(0, -9.81, 0));

    AddLayer(0);
}

void tmt::physics::PhysicalWorld::Update()
{
    dynamicsWorld->stepSimulation(1.0 / 6.0f, 1);

    dynamicsWorld->performDiscreteCollisionDetection();
    int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < nManifolds; i++)
    {
        btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject *obA = contactManifold->getBody0();
        const btCollisionObject *obB = contactManifold->getBody1();
        contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());
        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            // std::cout << "Collision between shapes " << obA->getCollisionShape()
            //	<< " and " << obB->getCollisionShape() << std::endl;

            PhysicsBody *goA = nullptr;
            PhysicsBody *goB = nullptr;

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

            auto poA = static_cast<particle::Particle *>(obA->getCollisionShape()->getUserPointer());
            auto poB = static_cast<particle::Particle *>(obB->getCollisionShape()->getUserPointer());

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
                btManifoldPoint &pt = contactManifold->getContactPoint(j);
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
                                                                          faceIdA, faceIdB](bool a) -> CollisionBase {
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

    doneFirstPhysicsUpdate = true;
}

void tmt::physics::PhysicalWorld::RemoveBody(int pid, int cpid)
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

short tmt::physics::PhysicalWorld::AddLayer(short mask)
{
    var l = 1 << layers.size();
    layerMasks.push_back(mask);
    layers.push_back(l);
    return l;
}

short tmt::physics::PhysicalWorld::AddLayer()
{
    var l = 1 << layers.size();

    short m = l;

    for (short layer : layers)
        m |= layer;
 
    layerMasks.push_back(m);
    layers.push_back(l);
    return l;
}

inline short tmt::physics::PhysicalWorld::GetLayer(short idx) { return layers[idx]; }


std::vector<tmt::physics::PhysicsBody *> tmt::physics::PhysicalWorld::GetGameObjectsCollidingWith(PhysicsBody *collider)
{
    {
        std::vector<PhysicsBody *> collisions;

        int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
        for (int i = 0; i < nManifolds; i++)
        {
            btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject *obA = contactManifold->getBody0();
            const btCollisionObject *obB = contactManifold->getBody1();

            auto goA = static_cast<PhysicsBody *>(obA->getCollisionShape()->getUserPointer());
            auto goB = static_cast<PhysicsBody *>(obB->getCollisionShape()->getUserPointer());
            if (goA != nullptr && goB != nullptr)
                std::cout << "Collision between " << goA->name << " and " << goB->name << std::endl;
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

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForBox(glm::vec3 bounds)
{

    var info = ColliderInitInfo();

    info.bounds = bounds;

    info.s = Box;

    return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForSphere(float radius)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.s = Sphere;

    return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForCapsule(float radius, float height)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.height = height;
    info.s = Capsule;

    return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForMesh(render::Mesh *mesh)
{
    var info = ColliderInitInfo();

    info.mesh = mesh;
    info.s = Mesh;

    return info;
}

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

tmt::physics::PhysicsBody::PhysicsBody(ColliderObject *collisionObj, float mass) : Object()
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
        rigidBody->setUserIndex2(physicalBodies.size());

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
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setLinearVelocity(convertVec3(v));
}

glm::vec3 tmt::physics::PhysicsBody::GetVelocity()
{
    if (pId >= physicalBodies.size())
        return glm::vec3{0};
    var pBody = physicalBodies[pId];

    return convertVec3(pBody->getLinearVelocity());
}

void tmt::physics::PhysicsBody::SetAngular(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setAngularVelocity(convertVec3(v));
}

void tmt::physics::PhysicsBody::AddImpulse(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->applyCentralImpulse(convertVec3(v));
}

void tmt::physics::PhysicsBody::AddForce(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->applyCentralForce(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetLinearFactor(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setLinearFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetAngularFactor(glm::vec3 v)
{
    if (pId >= physicalBodies.size())
        return;
    var pBody = physicalBodies[pId];

    pBody->setAngularFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetDamping(float linear, float angular)
{
    if (pId >= physicalBodies.size())
        return;
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

bool tmt::physics::OBB::Check(OBB* other, glm::vec3& collisionNormal, glm::vec3& collisionPoint)
{
    // Extract the properties of the OBBs
    const glm::vec3& centerA = this->center;
    const glm::vec3& centerB = other->center;
    const glm::vec3& halfExtentsA = this->halfSize;
    const glm::vec3& halfExtentsB = other->halfSize;
    const glm::mat3& axesA = this->axis;
    const glm::mat3& axesB = other->axis;

    // Compute the rotation matrix expressing B in A's coordinate frame
    glm::mat3 R;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            R[i][j] = glm::dot(axesA[i], axesB[j]);
        }
    }

    // Compute the translation vector
    glm::vec3 t = centerB - centerA;
    // Bring translation into A's coordinate frame
    t = glm::vec3(glm::dot(t, axesA[0]), glm::dot(t, axesA[1]), glm::dot(t, axesA[2]));

    // Compute common subexpressions
    glm::mat3 AbsR;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            AbsR[i][j] = std::abs(R[i][j]) + std::numeric_limits<float>::epsilon();
        }
    }

    // Test axes L = A0, A1, A2
    for (int i = 0; i < 3; ++i)
    {
        if (std::abs(t[i]) > halfExtentsA[i] + glm::dot(halfExtentsB, AbsR[i]))
        {
            collisionNormal = axesA[i];
            return false;
        }
    }

    // Test axes L = B0, B1, B2
    for (int i = 0; i < 3; ++i)
    {
        if (std::abs(glm::dot(t, glm::vec3(R[0][i], R[1][i], R[2][i]))) >
            glm::dot(halfExtentsA, glm::vec3(AbsR[0][i], AbsR[1][i], AbsR[2][i])) + halfExtentsB[i])
        {
            collisionNormal = axesB[i];
            return false;
        }
    }

    // Test axis L = A0 x B0
    if (std::abs(t[2] * R[1][0] - t[1] * R[2][0]) > halfExtentsA[1] * AbsR[2][0] + halfExtentsA[2] * AbsR[1][0] +
            halfExtentsB[1] * AbsR[0][2] + halfExtentsB[2] * AbsR[0][1])
    {
        collisionNormal = glm::cross(axesA[0], axesB[0]);
        return false;
    }

    // Test axis L = A0 x B1
    if (std::abs(t[2] * R[1][1] - t[1] * R[2][1]) > halfExtentsA[1] * AbsR[2][1] + halfExtentsA[2] * AbsR[1][1] +
            halfExtentsB[0] * AbsR[0][2] + halfExtentsB[2] * AbsR[0][0])
    {
        collisionNormal = glm::cross(axesA[0], axesB[1]);
        return false;
    }

    // Test axis L = A0 x B2
    if (std::abs(t[2] * R[1][2] - t[1] * R[2][2]) > halfExtentsA[1] * AbsR[2][2] + halfExtentsA[2] * AbsR[1][2] +
            halfExtentsB[0] * AbsR[0][1] + halfExtentsB[1] * AbsR[0][0])
    {
        collisionNormal = glm::cross(axesA[0], axesB[2]);
        return false;
    }

    // Test axis L = A1 x B0
    if (std::abs(t[0] * R[2][0] - t[2] * R[0][0]) > halfExtentsA[0] * AbsR[2][0] + halfExtentsA[2] * AbsR[0][0] +
            halfExtentsB[1] * AbsR[1][2] + halfExtentsB[2] * AbsR[1][1])
    {
        collisionNormal = glm::cross(axesA[1], axesB[0]);
        return false;
    }

    // Test axis L = A1 x B1
    if (std::abs(t[0] * R[2][1] - t[2] * R[0][1]) > halfExtentsA[0] * AbsR[2][1] + halfExtentsA[2] * AbsR[0][1] +
            halfExtentsB[0] * AbsR[1][2] + halfExtentsB[2] * AbsR[1][0])
    {
        collisionNormal = glm::cross(axesA[1], axesB[1]);
        return false;
    }

    // Test axis L = A1 x B2
    if (std::abs(t[0] * R[2][2] - t[2] * R[0][2]) > halfExtentsA[0] * AbsR[2][2] + halfExtentsA[2] * AbsR[0][2] +
            halfExtentsB[0] * AbsR[1][1] + halfExtentsB[1] * AbsR[1][0])
    {
        collisionNormal = glm::cross(axesA[1], axesB[2]);
        return false;
    }

    // Test axis L = A2 x B0
    if (std::abs(t[1] * R[0][0] - t[0] * R[1][0]) > halfExtentsA[0] * AbsR[1][0] + halfExtentsA[1] * AbsR[0][0] +
            halfExtentsB[1] * AbsR[2][2] + halfExtentsB[2] * AbsR[2][1])
    {
        collisionNormal = glm::cross(axesA[2], axesB[0]);
        return false;
    }

    // Test axis L = A2 x B1
    if (std::abs(t[1] * R[0][1] - t[0] * R[1][1]) > halfExtentsA[0] * AbsR[1][1] + halfExtentsA[1] * AbsR[0][1] +
            halfExtentsB[0] * AbsR[2][2] + halfExtentsB[2] * AbsR[2][0])
    {
        collisionNormal = glm::cross(axesA[2], axesB[1]);
        return false;
    }

    // Test axis L = A2 x B2
    if (std::abs(t[1] * R[0][2] - t[0] * R[1][2]) > halfExtentsA[0] * AbsR[1][2] + halfExtentsA[1] * AbsR[0][2] +
            halfExtentsB[0] * AbsR[2][1] + halfExtentsB[1] * AbsR[2][0])
    {
        collisionNormal = glm::cross(axesA[2], axesB[2]);
        return false;
    }

    // No separating axis found, the OBBs must be intersecting
    collisionNormal = glm::vec3(0.0f); // No specific normal axis

    // Calculate collision point
    collisionPoint = (centerA + centerB) * 0.5f;

    return true;
}

tmt::physics::OBB* tmt::physics::OBB::FromBox(glm::vec3 position, glm::vec3 size, glm::quat rotation)
{
    var box = new OBB;

    box->center = position;
    box->halfSize = size / 2.0f;
    box->axis = glm::mat3_cast(rotation);

    return box;
}
