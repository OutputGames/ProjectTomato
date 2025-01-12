#if !defined(_2D_HPP)
#define _2D_HPP

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

        void OnCollision(PhysicsCollision* col);

    private:
        friend PhysicsBody2D;
        friend PhysicsWorld2D;

        PhysicsBody2D* body;
    };

    struct BoxCollider2D : PhysicsCollider2D
    {
        glm::vec2 size;

    private:
        friend PhysicsWorld2D;
        ui::Rect rect;
    };

    struct PhysicsBody2D : obj::Object
    {
        tmt::physics::PhysicsBody::TransformRelationship Relationship = tmt::physics::PhysicsBody::Parent;

        glm::vec2 virtualPosition = glm::vec2(0);
        glm::vec2 velocity = glm::vec2(0);
        float mass = 1;

        PhysicsBody2D(PhysicsCollider2D* collider);

        void Update() override;

    private:
        friend struct PhysicsWorld2D;

        PhysicsCollider2D* collider;
    };

    struct PhysicsWorld2D
    {
        PhysicsWorld2D();
        ~PhysicsWorld2D();

        void Update();

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
