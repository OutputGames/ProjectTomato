#ifndef PARTICLECOLLISION_H
#define PARTICLECOLLISION_H

#include "utils.hpp" 
namespace tmt::particle {
 struct Particle;
 };
#include "tomato/Physics/CollisionBase.hpp"


namespace tmt::physics {

struct ParticleCollision : CollisionBase
{
    particle::Particle *other;
};

}

#endif