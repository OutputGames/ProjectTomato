#include "particle.hpp"
#include "globals.hpp"

glm::mat4 tmt::particle::Particle::getTransform()
{
    var p = position;
    var r = rotation;
    var s = scale;

    if (pId == -1)
    {
        p += emitterParent->GetGlobalPosition();
        r = emitterParent->GetGlobalRotation() * r;
    }

    var rt = toMat4(rotation);

    return translate(glm::mat4(1.0), p) * rt * glm::scale(glm::mat4(1.0), s);
}

glm::vec3 tmt::particle::Particle::GetUp()
{
    auto q = glm::quat(emitterParent->GetGlobalRotation() * rotation);

    return q * glm::vec3{0, 1, 0};
}

glm::vec3 tmt::particle::Particle::GetRight()
{
    auto q = glm::quat(emitterParent->GetGlobalRotation() * rotation);

    return q * glm::vec3{1, 0, 0};
}

glm::vec3 tmt::particle::Particle::GetForward()
{
    auto q = glm::quat((emitterParent->GetGlobalRotation() * rotation));

    return q * glm::vec3{0, 0, 1};
}

void tmt::particle::Particle::OnCollision(physics::Collision c)
{
    emitterParent->OnCollision(c, this);
}

void tmt::particle::Particle::OnParticleCollision(physics::ParticleCollision c)
{
}

tmt::particle::ParticleEmitter::ParticleEmitter() :
    Object()
{
    system = new ParticleSystem();

    system->renderer.material = new render::Material(defaultShader);
}

void tmt::particle::ParticleEmitter::Emit(int amount)
{
    if (particles.size() >= system->maxParticles)
        return;

    for (int i = 0; i < amount; ++i)
    {
        auto particle = new Particle();

        particle->lifetime = system->startLifetime;
        particle->scale = glm::vec3{system->startSize};

        particle->position = {0, 0, 0};
        particle->rotation = {1, 0, 0, 0};
        particle->emitterParent = this;
        particle->velocity = particle->GetForward();

        switch (system->shape.type)
        {
            case ParticleSystem::SystemShape::Cone:
            {
                float dir = randval(0, 360);
                float rad = randomFloat(0, system->shape.radius);

                // particle->position += glm::vec3{ glm::sin(dir) * rad,0,glm::cos(dir) * rad };
            }
            break;
        }

        if (system->collision.useColliders)
        {
#ifndef __SWITCH__
            particle->pId = physicalBodies.size();
            particle->_callback = physics::ParticleCollisionCallback();
            particle->_callback.particle = particle;

            physics::ColliderInitInfo info{system->collision.shape, particle->scale, particle->scale.x,
                                           particle->scale.y};

            var shape = ShapeFromInfo(info);

            shape->setUserPointer(particle);

            particle->cPID = collisionObjs.size();
            collisionObjs.push_back(shape);

            float mass = system->collision.mass;

            bool isDynamic = true;

            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                collisionObjs[particle->cPID]->calculateLocalInertia(mass, localInertia);

            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(convertVec3(particle->position + GetGlobalPosition()));
            startTransform.setRotation(convertQuat(GetGlobalRotation() * particle->rotation));

            auto myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionObjs[particle->cPID],
                                                            localInertia);

            var rigidBody = new btRigidBody(rbInfo);

            // rigidBody->setActivationState(DISABLE_DEACTIVATION);

            rigidBody->setUserPointer(particle);

            dynamicsWorld->addRigidBody(rigidBody);

            physicalBodies.push_back(rigidBody);

            managed_particles.push_back(particle);

            rigidBody->applyCentralImpulse(convertVec3(GetForward() * system->startSpeed));
#endif
        }
        else
        {
            particle->pId = -1;
        }

        particles.push_back(particle);
    }
}

void tmt::particle::ParticleEmitter::Update()
{
    float timeScale = 0.1f;

    if (time::getTime() <= 1)
    {
        isPlaying = system->playOnStart;
    }

    if (isPlaying)
    {
        time += time::getDeltaTime() * timeScale;
        time_alloc += time::getDeltaTime() * timeScale;

        if (time_alloc >= timeScale && system->emission.rateOverTime > 0)
        {
            Emit();
            time_alloc = 0;
        }

        if (time >= system->duration)
        {
            time = 0;
            if (!system->looping)
                isPlaying = false;
        }
    }

    std::vector<Particle*> deleteParticles;
    for (auto particle : particles)
    {
        if (system->collision.useColliders == false)
            particle->pId = -1;

        if (isPlaying)
        {
            particle->lifetime -= time::getDeltaTime();

#ifndef __SWITCH__
            if (particle->pId == -1)
            {
                particle->position += particle->velocity * system->startSpeed * time::getDeltaTime();
            }
            else
            {
                var body = physicalBodies[particle->pId];
                particle->position = convertVec3(body->getWorldTransform().getOrigin());
                particle->rotation = convertQuatEuler(body->getWorldTransform().getRotation());
                particle->velocity = convertVec3(body->getLinearVelocity());
            }
#endif

        }

        if (particle->lifetime <= 0)
        {
            deleteParticles.push_back(particle);
        }
        else
        {
            system->renderer.mesh->draw(particle->getTransform(), system->renderer.material, particle->position, 0);
        }
    }

    for (auto deleteParticle : deleteParticles)
    {
        particles.erase(std::find(particles.begin(), particles.end(), deleteParticle));
        managed_particles.erase(std::find(managed_particles.begin(), managed_particles.end(), deleteParticle));
        mainScene->physicsWorld->RemoveBody(deleteParticle->pId, deleteParticle->cPID);
    }
}

void tmt::particle::ParticleEmitter::OnCollision(physics::Collision c, Particle* p)
{
    p->lifetime -= system->collision.lifetimeLoss * system->startLifetime;
}
