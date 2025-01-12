#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

physics::PhysicsCollider2D::PhysicsCollider2D()
{
}

void physics::PhysicsCollider2D::OnCollision(PhysicsCollision* col)
{
    //std::cout << "Collision!" << std::endl;
}

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
            var p = glm::vec2(parent->position);

            parent->position = glm::vec3(virtualPosition, 0);

            var diff = p - virtualPosition;
            float tolerance = 1;
            if (glm::length(diff) > tolerance)
            {
                virtualPosition = p;
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
}

physics::PhysicsWorld2D::~PhysicsWorld2D()
{

}

void resolveCollision(tmt::ui::Rect& a, tmt::ui::Rect& b, glm::vec2 masses, glm::vec4 velocities)
{
    var aMax = a.getMax();
    var aMin = a.getMin();
    var bMax = b.getMax();
    var bMin = b.getMin();

    var aVelocity = glm::vec2{velocities.x, velocities.y};
    var bVelocity = glm::vec2{velocities.z, velocities.w};

    glm::vec2 overlap(std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x),
                      std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y));

    if (overlap.x < overlap.y)
    {
        float totalMass = masses.x + masses.y;
        float aMove = overlap.x * (masses.y / totalMass);
        float bMove = overlap.x * (masses.x / totalMass);

        if (aMin.x < bMin.x)
        {
            aMax.x -= aMove;
            bMin.x += bMove;
        }
        else
        {
            aMin.x += aMove;
            bMax.x -= bMove;
        }

        if (aVelocity.y != 0.0f)
        {
            aMin.y += aVelocity.y;
            aMax.y += aVelocity.y;
        }
        if (bVelocity.y != 0.0f)
        {
            bMin.y += bVelocity.y;
            bMax.y += bVelocity.y;
        }
    }
    else
    {
        float totalMass = masses.x + masses.y;
        float aMove = overlap.y * (masses.y / totalMass);
        float bMove = overlap.y * (masses.x / totalMass);

        if (aMin.y < bMin.y)
        {
            aMax.y -= aMove;
            bMin.y += bMove;
        }
        else
        {
            aMin.y += aMove;
            bMax.y -= bMove;
        }

        if (aVelocity.x != 0.0f)
        {
            aMin.x += aVelocity.x;
            aMax.x += aVelocity.x;
        }
        if (bVelocity.x != 0.0f)
        {
            bMin.x += bVelocity.x;
            bMax.x += bVelocity.x;
        }
    }

    a.CopyMinMax(aMin, aMax);
    b.CopyMinMax(bMin, bMax);
}

void physics::PhysicsWorld2D::Update()
{

    float timeStep = 1.0f / 60.0f;
    auto gravity = glm::vec2(0, -9.81);

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

            rect.width = box->size.x;
            rect.height = box->size.y;
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
                                resolveCollision(box->rect, _box->rect, glm::vec2(box->body->mass, _box->body->mass),
                                                 glm::vec4(box->body->velocity, _box->body->velocity));

                                box->body->velocity = glm::vec2(0);
                                _box->body->velocity = glm::vec2(0);
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
        if (physicsCollider2D->body->mass <= 0)
            continue;

        var box = physicsCollider2D->Cast<BoxCollider2D>();
        if (box)
        {
            var rect = box->rect;


            physicsCollider2D->body->virtualPosition = glm::vec2(rect.x, rect.y);
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
