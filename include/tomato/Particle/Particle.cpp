#include "Particle.hpp" 
#include "globals.cpp" 

glm::mat4 tmt::particle::Particle::getTransform()
{
    var p = position;
    var r = rotation;
    var s = scale;

    if (pId == -1)
    {
        p += emitterParent->GetGlobalPosition();
        p += emitterParent->GetGlobalRotation();
    }

    var rx = rotate(glm::mat4(1.0), glm::radians(r.x), {1, 0, 0});
    var ry = rotate(glm::mat4(1.0), glm::radians(r.y), {0, 1, 0});
    var rz = rotate(glm::mat4(1.0), glm::radians(r.z), {0, 0, 1});

    var rt = ry * rx * rz;

    return translate(glm::mat4(1.0), p) * rt * glm::scale(glm::mat4(1.0), s);
}

glm::vec3 tmt::particle::Particle::GetUp()
{
    auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

    return q * glm::vec3{0, 1, 0};
}

glm::vec3 tmt::particle::Particle::GetRight()
{
    auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

    return q * glm::vec3{1, 0, 0};
}

glm::vec3 tmt::particle::Particle::GetForward()
{
    auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

    return q * glm::vec3{0, 0, 1};
}

void tmt::particle::Particle::OnCollision(physics::Collision c)
{
    emitterParent->OnCollision(c, this);
}

void tmt::particle::Particle::OnParticleCollision(physics::ParticleCollision c)
{
}

