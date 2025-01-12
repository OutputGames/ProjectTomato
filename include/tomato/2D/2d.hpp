#if !defined(_2D_HPP)
#define _2D_HPP
#include "Obj/obj.hpp"

namespace tmt::engine2D::physics
{
    struct PhysicsCollider2D : obj::Object
    {
        PhysicsCollider2D();
    };

    struct BoxCollider2D : PhysicsCollider2D
    {

    };

    struct PhysicsBody2D : obj::Object
    {
        PhysicsBody2D();

        void Update() override;

    private:
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
    };

    void init();
    void update();
    void shutdown();
    bool setup();

}


#endif // 2D_HPP
