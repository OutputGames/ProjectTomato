#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils.hpp"
#include "tomato/Obj/obj.hpp"
#include "tomato/Obj/obj.hpp"


namespace tmt::render
{
    struct Mesh;
}

namespace tmt::render
{
    struct Mesh;
}

namespace tmt::particle
{
    struct Particle;
}

namespace tmt::physics
{
    struct PhysicsBody;
}

namespace tmt::physics
{
    struct CollisionCallback;
}

namespace tmt::physics
{
    struct PhysicalWorld;
}

namespace tmt::physics
{
    struct CollisionCallback;
}


namespace tmt::physics
{

    struct CollisionCallback;
    struct PhysicalWorld;
    enum CollisionShape;
    struct ColliderInitInfo;
    struct CollisionBase;
    struct Collision;
    struct ParticleCollision;
    struct ColliderObject;
    struct PhysicsBody;
    struct Ray;
    struct RaycastHit;

    struct CollisionCallback : btCollisionWorld::ContactResultCallback
    {
        PhysicsBody* body;
        ColliderObject* collider;

        // Overriding the callback method
        btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0,
                                 int index0,
                                 const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override;
    };

    struct ParticleCollisionCallback : btCollisionWorld::ContactResultCallback
    {
        particle::Particle* particle;

        // Overriding the callback method
        btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0,
                                 int index0,
                                 const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override;
    };

    struct PhysicalWorld
    {
        PhysicalWorld();
        ~PhysicalWorld();

        void Update();

        void RemoveBody(int pid, int cpid);

        std::vector<PhysicsBody*> GetGameObjectsCollidingWith(PhysicsBody* collider);

        void SetLayerMask(int layer, std::vector<int> mask);

        std::vector<int> layerMasks;
    };


    enum CollisionShape
    {
        Box,
        Sphere,
        Capsule,
        Mesh
    };

    struct ColliderInitInfo
    {
        CollisionShape s;
        glm::vec3 bounds;
        float radius;
        float height;
        render::Mesh* mesh;

        static ColliderInitInfo ForBox(glm::vec3 bounds);
        static ColliderInitInfo ForSphere(float radius);
        static ColliderInitInfo ForCapsule(float radius, float height);
        static ColliderInitInfo ForMesh(render::Mesh* mesh);
    };

    struct CollisionBase
    {
        glm::vec3 contactPoint;
        glm::vec3 normal;
        int faceId;
    };

    struct Collision : CollisionBase
    {
        PhysicsBody* other;
    };

    struct ParticleCollision : CollisionBase
    {
        particle::Particle* other;
    };

    struct ColliderObject : obj::Object
    {
        ColliderInitInfo initInfo;
        bool scaleByObject = false;
        ColliderObject(ColliderInitInfo info, Object* parent = nullptr);

    private:
        friend struct PhysicsBody;
        friend CollisionCallback;

        u16 pId = -1;
    };

    struct PhysicsBody : obj::Object
    {
        float mass = 1;
        int layer = 0;

        PhysicsBody(ColliderObject* collisionObj, float mass = 1);

        enum TransformRelationship
        {
            Self = 0,
            Parent
        } transRelation = Self;

        void Update() override;

        void SetVelocity(glm::vec3 v);
        glm::vec3 GetVelocity();

        void SetPushVelocity(glm::vec3 v);

        void SetAngular(glm::vec3 v);

        void AddImpulse(glm::vec3 v);
        void AddForce(glm::vec3 v);
        void SetForward(glm::vec3 v);

        void SetLinearFactor(glm::vec3 v);
        void SetAngularFactor(glm::vec3 v);

        void SetDamping(float linear, float angular);

        void AddCollisionEvent(std::function<void(Collision)> func);
        void AddParticleCollisionEvent(std::function<void(ParticleCollision)> func);

        glm::vec3 GetBasisColumn(float v);
        glm::vec3 GetBasisRow(float v);
        void Reset();

        ~PhysicsBody() override;

    private:
        friend PhysicalWorld;
        friend CollisionCallback;

        CollisionCallback callback_;

        u16 pId = UINT16_MAX;
        u16 cPID = UINT16_MAX;

        std::vector<std::function<void(Collision)>> collisionEvents;
        std::vector<std::function<void(ParticleCollision)>> particleCollisionEvents;

        void OnCollision(Collision c);
        void OnParticleCollision(ParticleCollision c);
    };


    struct Ray
    {
        glm::vec3 position, direction;
        float maxDistance = 10000;
        int layer = 0;

        RaycastHit* Cast();
    };

    struct RaycastHit
    {
        glm::vec3 point, normal;
        PhysicsBody* hit;
    };

    struct OBB
    {
        glm::vec3 center, velocity = {0, 0, 0};
        glm::vec3 halfSize;
        glm::mat3 axis;

        int layer = 0;
        void* userData = nullptr;
        bool isStatic = false;

        OBB();

        bool Check(OBB* other, glm::vec3& mtv);


        static OBB* FromBox(glm::vec3 position, glm::vec3 size, glm::quat rotation);
    };

}

#endif
