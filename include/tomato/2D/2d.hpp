#if !defined(_2D_HPP)
#define _2D_HPP

#include <box2d/box2d.h>

#include "tomato/Obj/obj.hpp"
#include "tomato/Physics/physics.hpp"
#include "tomato/Ui/ui.hpp"

namespace tmt::physics
{
    struct PhysicsBody;
}

namespace tmt::engine2D::physics
{
    struct PhysicsBody2D;
    struct PhysicsCollider2D;

    struct PhysicsCollision
    {
        PhysicsCollider2D* other;

    };

    struct PhysicsCollider2D : obj::Object
    {
        PhysicsCollider2D();

        void OnCollision(PhysicsCollision col);
        virtual void BindToBody(b2BodyId id);
        virtual void DestroyShape();

    private:
        friend PhysicsBody2D;
        friend PhysicsWorld2D;

        PhysicsBody2D* body;
    };

    struct BoxCollider2D : PhysicsCollider2D
    {
        BoxCollider2D();
        glm::vec2 size;
        void BindToBody(b2BodyId id) override;
        void DestroyShape() override;

        void Update() override;

    private:
        friend PhysicsWorld2D;
        ui::Rect rect;
        b2Polygon box;
    };

    struct PolygonCollider2D : PhysicsCollider2D
    {
        std::vector<glm::vec2> points;
        void BindToBody(b2BodyId id) override;

    private:
        friend PhysicsWorld2D;

    };

    inline b2Vec2 cvtv2(glm::vec2 v) { return b2Vec2(v.x, v.y); }
    inline glm::vec2 cvtv2(b2Vec2 v) { return glm::vec2(v.x, v.y); }

    struct PhysicsBody2D : obj::Object
    {
        tmt::physics::PhysicsBody::TransformRelationship Relationship = tmt::physics::PhysicsBody::Parent;

        glm::vec2 virtualPosition = glm::vec2(0);
        glm::vec2 velocity = glm::vec2(0);
        float mass = 1;

        PhysicsBody2D(PhysicsCollider2D* collider);
        ~PhysicsBody2D() override;

        void ApplyImpulse(glm::vec2 i);

        void Update() override;

    private:
        friend struct PhysicsWorld2D;

        PhysicsCollider2D* collider;
        b2BodyId id;
    };

    struct PhysicsWorld2D
    {
        PhysicsWorld2D();
        ~PhysicsWorld2D();

        void Update();

        b2WorldId id;

    private:
        friend PhysicsBody2D;

        std::vector<PhysicsBody2D*> bodies;
        std::vector<PhysicsCollider2D*> colliders;
    };

    void init();
    void update();
    void shutdown();

}


#endif // 2D_HPP
