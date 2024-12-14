#include "PhysicalWorld.hpp" 
#include "globals.cpp" 

tmt::physics::PhysicalWorld::PhysicalWorld()
{
    auto configuration = new btDefaultCollisionConfiguration();

    auto dispatcher = new btCollisionDispatcher(configuration);

    btBroadphaseInterface *overlappingPairCache = new btDbvtBroadphase();

    auto solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);

    dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
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
                var faceIdA = -1;
                var faceIdB = -1;

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

