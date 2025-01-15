#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

static float timeStep = 1.0f / 60.0f;

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
                //virtualPosition = p;
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

std::vector<glm::vec2> findAxes(const physics::PolygonCollider2D* poly)
{
    std::vector<glm::vec2> axes;
    const var& vertices = poly->points;

    for (int i = 0; i < vertices.size(); i++)
    {
        var edge = vertices[(i + 1) % vertices.size()] - vertices[i];
        axes.push_back({-edge.y, edge.x});
    }

    return axes;
}

std::pair<float, float> projectPolygon(const glm::vec2& axis, const std::vector<glm::vec2>& vertices)
{
    float min = std::numeric_limits<float>::infinity();
    float max = -std::numeric_limits<float>::infinity();

    for (const auto& vertex : vertices)
    {
        float projection = glm::dot(vertex, axis);
        min = std::min(min, projection);
        max = std::max(max, projection);
    }

    return {min, max};
}

bool physics::PolygonCollider2D::CheckCollision(ui::Rect r)
{
    if (points.size() == 0)
        return false;

    int next = 0;

    for (int i = 0; i < points.size(); i++)
    {
        next = i + 1;
        if (next == points.size())
            next = 0;

        glm::vec2 c = points[i] + glm::vec2(GetGlobalPosition());
        var n = points[next] + glm::vec2(GetGlobalPosition());

        bool collision = r.isLineOnRect(c, n);

        if (collision)
            return true;
    }

    return false;
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

void resolveCollision(tmt::ui::Rect& a, physics::PolygonCollider2D* poly, glm::vec2 masses, glm::vec4 velocities)
{
    var min = a.getMin();
    var max = a.getMax();

    std::vector<glm::vec2> vertices = {min, {max.x, min.y}, max, {min.x, max.y}};

    std::vector<glm::vec2> axes = {{1, 0}, {0, 1}};

    var polyAxes = findAxes(poly);
    axes.insert(axes.end(), polyAxes.begin(), polyAxes.end());

    float minOverlap = std::numeric_limits<float>::infinity();
    glm::vec2 mtvAxis;

    for (const auto& axis : axes)
    {
        var [minA, maxA] = projectPolygon(axis, vertices);
        var [minB, maxB] = projectPolygon(axis, poly->points);

        float overlapValue = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlapValue < minOverlap)
        {
            minOverlap = overlapValue;
            mtvAxis = axis;
        }
    }

    float totalMass = masses.x + masses.y;

    mtvAxis *= minOverlap;

    var aVelocity = glm::vec2{velocities.x, velocities.y};
    var bVelocity = glm::vec2{velocities.z, velocities.w};

    min += (mtvAxis * (masses.x / totalMass)) + aVelocity;
    max += (mtvAxis * (masses.x / totalMass)) + aVelocity;

    poly->position += glm::vec3((mtvAxis * (masses.y / totalMass)) + bVelocity, 0);

    a.CopyMinMax(min, max);
}

void physics::PhysicsWorld2D::Update()
{
    auto gravity = glm::vec2(0, -25);

    for (auto physicsBody2D : bodies)
    {
        //physicsBody2D->velocity = glm::vec2(0);
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
            var poly = col->Cast<PolygonCollider2D>();

            for (auto collider : colliders)
            {
                if (collider != col)
                {
                    var _box = collider->Cast<BoxCollider2D>();
                    var _poly = collider->Cast<PolygonCollider2D>();

                    if (box)
                    {
                        if (_box)
                        {
                            var _col = box->rect.isRectColliding(_box->rect);
                            if (_col)
                            {
                                resolveCollision(box->rect, _box->rect, glm::vec2(box->body->mass, _box->body->mass),
                                                 glm::vec4(box->body->velocity * timeStep,
                                                           _box->body->velocity * timeStep));

                                box->body->velocity = glm::vec2(0);
                                _box->body->velocity = glm::vec2(0);
                                box->OnCollision(new PhysicsCollision{_box});
                                _box->OnCollision(new PhysicsCollision{box});
                            }
                        }

                        if (_poly)
                        {
                            var _col = _poly->CheckCollision(box->rect);
                            if (_col)
                            {
                                resolveCollision(box->rect, _poly, glm::vec2(box->body->mass, _poly->body->mass),
                                                 glm::vec4(box->body->velocity * timeStep,
                                                           _poly->body->velocity * timeStep));

                                box->body->velocity = glm::vec2(0);
                                _poly->body->velocity = glm::vec2(0);
                                box->OnCollision(new PhysicsCollision{_poly});
                                _poly->OnCollision(new PhysicsCollision{box});
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
