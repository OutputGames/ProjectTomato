#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

static float timeStep = 1.0f / 60.0f;

physics::PhysicsCollider2D::PhysicsCollider2D()
{
}

void physics::PhysicsCollider2D::OnCollision(PhysicsCollision col)
{
    //std::cout << "Collision!" << std::endl;
}

void physics::PhysicsCollider2D::BindToBody(b2BodyId id)
{

}

void physics::PhysicsCollider2D::DestroyShape()
{
}

physics::BoxCollider2D::BoxCollider2D()
{

}

void physics::BoxCollider2D::BindToBody(b2BodyId id)
{
    var box = b2MakeBox(size.x / 2, size.y / 2);
    var shape = b2DefaultShapeDef();
    shape.density = 1.0f;
    shape.friction = 0.3f;

    b2CreatePolygonShape(id, &shape, &box);

    PhysicsCollider2D::BindToBody(id);
}

void physics::BoxCollider2D::DestroyShape()
{

    PhysicsCollider2D::DestroyShape();
}

void physics::BoxCollider2D::Update()
{

    PhysicsCollider2D::Update();
}

void physics::PolygonCollider2D::BindToBody(b2BodyId id)
{
    PhysicsCollider2D::BindToBody(id);
}

physics::PhysicsBody2D::PhysicsBody2D(PhysicsCollider2D* collider)
{
    collider->SetParent(this);
    collider->body = this;

    this->collider = collider;
}

physics::PhysicsBody2D::~PhysicsBody2D()
{

    b2DestroyBody(id);
}

void physics::PhysicsBody2D::Update()
{
    if (!b2Body_IsValid(id))
    {
        var def = b2DefaultBodyDef();

        def.position = cvtv2(virtualPosition);

        id = b2CreateBody(mainScene->physicsWorld2D->id, &def);

        collider->BindToBody(id);
    }

    b2Body_SetType(id, mass > 0 ? b2_dynamicBody : b2_staticBody);
    b2Body_SetLinearVelocity(id, cvtv2(velocity));
    virtualPosition = cvtv2(b2Body_GetPosition(id));
    if (Relationship == tmt::physics::PhysicsBody::Parent)
    {
        if (parent)
        {
            var p = glm::vec2(parent->position);

            parent->position = glm::vec3(virtualPosition, 0);

            var diff = p - virtualPosition;
            float tolerance = 1;
            if (glm::length(diff) > tolerance)
            {
                virtualPosition = p;
                b2Body_SetTransform(id, cvtv2(virtualPosition), b2Body_GetRotation(id));
            }
        }
    }
    else
    {
        position = glm::vec3(virtualPosition, 0);
    }
    Object::Update();
}

physics::PhysicsWorld2D::PhysicsWorld2D()
{
    var def = b2DefaultWorldDef();
    id = b2CreateWorld(&def);
}

physics::PhysicsWorld2D::~PhysicsWorld2D()
{
    b2DestroyWorld(id);
}

void physics::PhysicsWorld2D::Update()
{
    auto gravity = glm::vec2(0, -15);

    b2World_SetGravity(id, cvtv2(gravity));
    b2World_Step(id, timeStep, 4);
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
