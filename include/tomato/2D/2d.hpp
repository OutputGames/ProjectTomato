#if !defined(_2D_HPP)
#define _2D_HPP
#include "Obj/obj.hpp"
#include "Physics/physics.hpp"
#include "Ui/ui.hpp"

namespace tmt::physics
{
    struct PhysicsBody;
}

namespace tmt::engine2D::physics
{
    struct PhysicsCollider2D;

    struct PhysicsCollision
    {
        PhysicsCollider2D* other;

    };

    struct PhysicsCollider2D : obj::Object
    {
        PhysicsCollider2D();

        void OnCollision(PhysicsCollider2D* other);
    };

    struct BoxCollider2D : PhysicsCollider2D
    {
        ui::Rect rect;
    };

    struct PhysicsBody2D : obj::Object
    {
        tmt::physics::PhysicsBody::TransformRelationship Relationship;

        PhysicsBody2D();

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
    bool setup();

}


#endif // 2D_HPP
