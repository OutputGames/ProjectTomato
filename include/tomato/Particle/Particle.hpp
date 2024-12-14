#ifndef PARTICLE_H
#define PARTICLE_H

#include "utils.hpp" 
#include "tomato/Render/render.hpp"
#include "tomato/Physics/physics.hpp"
#include "tomato/Render/render.hpp"
#include "tomato/Physics/physics.hpp"
#include "tomato/Physics/physics.hpp"
#include "tomato/Obj/obj.hpp"
#include "tomato/Physics/physics.hpp"


namespace tmt::particle {
 struct Particle;
 }


namespace tmt::particle {

struct ParticleSystem;
struct Particle;
struct ParticleEmitter;

struct ParticleSystem
{
    float duration = 5;
    bool looping = true;

    float startSpeed = 1;
    float startSize = 0.1;
    float startLifetime = 5;

    render::Color startColor;

    struct SystemEmission
    {
        float rateOverTime = 10;
    } emission;

    struct SystemShape
    {
        enum Shape
        {
            Cone
        } type;

        float radius = 1;
        float angle = 35;
    } shape;

    struct SystemCollision
    {
        bool useColliders;
        physics::CollisionShape shape;
        float mass = 1;

        float lifetimeLoss;
    } collision;

    struct SystemRenderer
    {
        enum Mode
        {
            Mesh
        } mode;

        render::Mesh *mesh = GetPrimitive(prim::Sphere);

        render::Material *material;
    } renderer;

    float maxParticles = 100;

    bool playOnStart = true;
};

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
;

}

#endif