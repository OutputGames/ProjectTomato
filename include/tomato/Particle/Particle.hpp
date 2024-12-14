#ifndef PARTICLE_H
#define PARTICLE_H

#include "utils.hpp" 
#include "tomato/Render/Color.hpp"
#include "tomato/Particle/ParticleEmitter.hpp"
#include "tomato/Physics/ParticleCollision.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/ParticleCollision.hpp"


namespace tmt::particle {

struct Particle
{
    glm::vec3 position, rotation, scale;
    glm::vec3 velocity;
    render::Color color;
    float lifetime = 0;

    glm::mat4 getTransform();

    glm::vec3 GetUp();
    glm::vec3 GetRight();
    glm::vec3 GetForward();

    ParticleEmitter *emitterParent;

    void OnCollision(physics::Collision c);
    void OnParticleCollision(physics::ParticleCollision c);

    int pId = -1;
    u16 cPID = -1;
};

}

#endif