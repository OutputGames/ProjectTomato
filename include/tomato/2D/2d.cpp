#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

physics::PhysicsBody2D::PhysicsBody2D()
{
    collider = GetObjectFromType<PhysicsCollider2D>();

    mainScene->physicsWorld2D->bodies.push_back(this);
    mainScene->physicsWorld2D->colliders.push_back(collider);
}

void physics::PhysicsBody2D::Update()
{


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

                        }
                    }
                }
            }
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

bool physics::setup()
{
}
