#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

physics::PhysicsCollider2D::PhysicsCollider2D()
{
}

void physics::PhysicsCollider2D::OnCollision(PhysicsCollision* col) { std::cout << "Collision!" << std::endl; }

physics::PhysicsBody2D::PhysicsBody2D(PhysicsCollider2D* collider)
{
    collider->SetParent(this);
    collider->body = this;

    this->collider = collider;

    mainScene->physicsWorld2D->bodies.push_back(this);
    mainScene->physicsWorld2D->colliders.push_back(collider);
}

void physics::PhysicsBody2D::Update()
{


    if (Relationship == tmt::physics::PhysicsBody::Parent)
    {
        if (parent)
        {
            parent->position = virtualPosition;
        }
    }
    else
    {
        position = virtualPosition;
    }

    Object::Update();
}

physics::PhysicsWorld2D::PhysicsWorld2D()
{
}

physics::PhysicsWorld2D::~PhysicsWorld2D()
{

}


void physics::PhysicsWorld2D::Update()
{

    float timeStep = 1.0f / 60.0f;
    auto gravity = glm::vec3(0, -9.81, 0);

    for (auto physicsBody2D : bodies)
    {
        if (physicsBody2D->mass > 0)
        {
            physicsBody2D->virtualPosition += physicsBody2D->velocity * timeStep;
            physicsBody2D->velocity += gravity * timeStep;
        }
    }

    for (auto physicsCollider2D : colliders)
    {
        var box = physicsCollider2D->Cast<BoxCollider2D>();
        if (box)
        {
            var rect = ui::Rect{};
            rect.x = box->body->virtualPosition.x;
            rect.y = box->body->virtualPosition.y;

            rect.width = box->GetGlobalScale().x;
            rect.height = box->GetGlobalScale().y;
            box->rect = rect;
        }
    }

    for (auto body : bodies)
    {
        var col = body->collider;

        if (col)
        {
            var box = col->Cast<BoxCollider2D>();

            for (auto collider : colliders)
            {
                if (collider != col)
                {
                    var _box = collider->Cast<BoxCollider2D>();

                    if (box)
                    {
                        if (_box)
                        {
                            var col = box->rect.isRectColliding(_box->rect);
                            if (col)
                            {
                                box->rect.resolveCollision(_box->rect, glm::vec2(box->body->mass, _box->body->mass));
                                box->OnCollision(new PhysicsCollision{_box});
                                _box->OnCollision(new PhysicsCollision{box});
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto physicsCollider2D : colliders)
    {
        var box = physicsCollider2D->Cast<BoxCollider2D>();
        if (box)
        {
            var rect = box->rect;


            physicsCollider2D->body->virtualPosition = glm::vec3(rect.x, rect.y, 0);
        }
    }
}

void physics::init()
{

}

void physics::update()
{

}

void physics::shutdown()
{
}
