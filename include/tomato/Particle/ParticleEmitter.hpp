#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include "utils.hpp" 
namespace tmt::particle {
 struct Particle;
 };
#include "tomato/Particle/ParticleSystem.hpp"
#include "tomato/Obj/Object.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/ParticleCollision.hpp"


namespace tmt::particle {

struct ParticleEmitter : obj::Object
{
    ParticleSystem *system;

    ParticleEmitter();

    void Emit(int amount = 1);
    void Update() override;

  private:
    friend Particle;

    std::vector<Particle *> particles;
    float time = 0;
    bool isPlaying = false;
    float time_alloc = 0;

    std::vector<std::function<void(physics::Collision)>> collisionEvents;

    void OnCollision(physics::Collision c, Particle *p);
};

}

#endif