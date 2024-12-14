#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "utils.hpp" 
#include "tomato/Render/Color.hpp"
#include "tomato/Physics/CollisionShape.hpp"
#include "tomato/Render/Mesh.hpp"
#include "tomato/Render/Material.hpp"
#include "tomato/prim/PrimitiveType.hpp"


namespace tmt::particle {

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

}

#endif