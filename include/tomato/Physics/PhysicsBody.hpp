#ifndef PHYSICSBODY_H
#define PHYSICSBODY_H

#include "utils.hpp" 
#include "tomato/Physics/CollisionCallback.hpp"
#include "tomato/Obj/Object.hpp"
#include "tomato/Physics/ParticleCollision.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/ColliderObject.hpp"
#include "tomato/Physics/ColliderObject.hpp"
#include "tomato/Physics/Collision.hpp"
#include "tomato/Physics/ParticleCollision.hpp"


namespace tmt::physics {

struct PhysicsBody : obj::Object
{
    float mass = 1;

    PhysicsBody(ColliderObject *collisionObj, float mass = 1);

    enum TransformRelationship
    {
        Self,
        Parent
    } transRelation = Self;

    void Update() override;

    void SetVelocity(glm::vec3 v);
    glm::vec3 GetVelocity();

    void SetAngular(glm::vec3 v);

    void AddImpulse(glm::vec3 v);
    void AddForce(glm::vec3 v);

    void SetLinearFactor(glm::vec3 v);
    void SetAngularFactor(glm::vec3 v);

    void SetDamping(float linear, float angular);

    void AddCollisionEvent(std::function<void(Collision)> func);
    void AddParticleCollisionEvent(std::function<void(ParticleCollision)> func);

    glm::vec3 GetBasisColumn(float v);
    glm::vec3 GetBasisRow(float v);

  private:
    friend PhysicalWorld;
    friend CollisionCallback;

    CollisionCallback *callback_;

    u16 pId = -1;
    u16 cPID = -1;

    std::vector<std::function<void(Collision)>> collisionEvents;
    std::vector<std::function<void(ParticleCollision)>> particleCollisionEvents;

    void OnCollision(Collision c);
    void OnParticleCollision(ParticleCollision c);
};

}

#endif