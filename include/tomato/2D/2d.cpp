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
    // std::cout << "Collision!" << std::endl;
}

physics::PhysicsBody2D::PhysicsBody2D(PhysicsCollider2D* collider)
{
    collider->SetParent(this);
    collider->body = this;

    this->collider = collider;

    mainScene->physicsWorld2D->bodies.push_back(this);
    mainScene->physicsWorld2D->colliders.push_back(collider);
}

physics::PhysicsBody2D::~PhysicsBody2D()
{
    mainScene->physicsWorld2D->bodies.erase(
        std::find(mainScene->physicsWorld2D->bodies.begin(), mainScene->physicsWorld2D->bodies.end(), this));
    mainScene->physicsWorld2D->colliders.erase(
        std::find(mainScene->physicsWorld2D->colliders.begin(), mainScene->physicsWorld2D->colliders.end(), collider));
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
                // virtualPosition = p;
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
    std::vector<int> mask;

    for (int i = 0; i < 64; ++i)
    {
        mask.push_back(BIT(i));
    }

    SetLayerMask(BIT(0), mask);
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

void physics::PhysicsWorld2D::resolveCollision(BoxCollider2D* boxA, BoxCollider2D* boxB,
                                               BoxCollider2D::PreCollsiionData& data)
{
    ui::Rect& a = boxA->rect;
    ui::Rect& b = boxB->rect;

    var aMax = a.getMax();
    var aMin = a.getMin();
    var bMax = b.getMax();
    var bMin = b.getMin();

    var aVelocity = boxA->body->velocity;
    //var bVelocity = boxB->body->velocity;

    //var aMass = boxA->body->mass;
    //var bMass = boxB->body->mass;

    //if (!motion)
    //return;

    // check the X axis
    float oldRight = aMax.x;
    float oldBottom = aMax.y;

    var motion = aVelocity;
    bool hitSide = data.hitSide, hitTop = data.hitTop, hitBottom = data.hitBottom;

    ui::Rect newrect = a;
    newrect.x += motion.x;
    newrect.y += motion.y;

    if (!newrect.isRectColliding(b))
        return;

    var newMin = newrect.getMin();
    var newMax = newrect.getMax();

    bool canHitX = true;
    if (newMin.y < bMax.y) // our top is below wall bottom
        canHitX = false;
    else if (newMax.y >= bMin.y) // our bottom is over the wall top
        canHitX = false;

    if (canHitX)
    {
        float newRight = newMax.x;
        float objectRight = bMax.x;

        // check the box moving to the right
        // if we were outside the left wall before, and are not now, we hit something
        if (motion.x < 0)
        {
            if (oldRight >= bMin.x)
            {
                if (newRight < bMin.x)
                {
                    // we hit moving right, so set us back to where we hit the wall
                    newrect.x = bMin.x + (a.width / 2);
                    hitSide = true;
                }
            }
        }

        if (motion.x > 0)
        {
            // check the box moving to the left
            // if we were outside the right wall before, and are not now, we hit something
            if (aMin.x <= objectRight)
            {
                if (newMin.x > objectRight)
                {
                    // we hit moving left, so set us back to where we hit the wall
                    newrect.x = objectRight - (a.width / 2);
                    hitSide = true;
                }
            }
        }
    }

    newMin = newrect.getMin();
    newMax = newrect.getMax();

    // do the same for Y
    bool canHitY = true;
    if (newMin.x < bMax.x) // our left is past wall right
        canHitY = false;
    else if (newMax.x > bMin.x) // our right is past wall left
        canHitY = false;

    if (canHitY)
    {
        float newBottom = newMax.y;
        float objectBottom = bMax.y;

        if (motion.y <= 0)
        {
            if (oldBottom >= bMin.y)
            {
                if (newBottom < bMin.y)
                {
                    // we hit moving down, so set us back to where we hit the wall
                    newrect.y = bMin.y + (a.height / 2);
                    hitBottom = true;
                }
                else if (newBottom == bMin.y)
                {
                    hitBottom = true;
                }
            }
        }
        else if (motion.y > 0)
        {
            // check the box moving up
            // if we were outside the bottom wall before, and are not now, we hit something
            if (aMin.y <= objectBottom)
            {
                if (newMin.y > objectBottom)
                {
                    // we hit moving up, so set us back to where we hit the wall
                    newrect.y = bMax.y - (a.height / 2);
                    hitTop = true;
                }
            }
        }
    }

    newMin = newrect.getMin();
    newMax = newrect.getMax();
    aMax = a.getMax();
    aMin = a.getMin();


    data.hitBottom = hitBottom;
    data.hitSide = hitSide;
    data.hitTop = hitTop;

    boxA->body->velocity = (newMin - aMin) * 1.0f;

    //a.CopyMinMax(newrect.getMin(), newrect.getMax());
}

void physics::PhysicsWorld2D::Update()
{

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

        if (col && body->mass > 0 && body->GetActive())
        {
            var box = col->Cast<BoxCollider2D>();
            var pcd = BoxCollider2D::PreCollsiionData();
            pcd.preVelo = body->velocity;

            body->velocity *= timeStep;

            var layerMask = layerMasks[body->layer];

            for (auto collider : colliders)
            {
                if (collider != col && collider->GetActive() && layerMask & collider->body->layer)
                {
                    var _box = collider->Cast<BoxCollider2D>();

                    if (box)
                    {
                        if (_box)
                        {
                            resolveCollision(box, _box, pcd);

                            if (pcd.hitBottom || pcd.hitSide || pcd.hitTop)
                            {

                                box->OnCollision(PhysicsCollision{_box});
                                _box->OnCollision(PhysicsCollision{box});
                            }
                        }
                    }
                }
            }

            body->velocity *= 60;

            body->doGravity = !pcd.hitBottom;
            pcd.preVelo = glm::vec2(0);
            box->lastCollisionData = pcd;
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

    auto gravity = glm::vec2(0, -50);

    for (auto physicsBody2D : bodies)
    {
        // physicsBody2D->velocity = glm::vec2(0);
        if (physicsBody2D->mass > 0 && physicsBody2D->GetActive())
        {
            physicsBody2D->virtualPosition += physicsBody2D->velocity * timeStep;
            if (physicsBody2D->doGravity)
                physicsBody2D->velocity += gravity * (timeStep * 2);
        }
    }
}

void physics::PhysicsWorld2D::SetLayerMask(int layer, std::vector<int> mask)
{
    if (layerMasks.size() <= layer)
    {
        layerMasks.resize(layer + 1);
    }

    int m = 0;

    for (int value : mask)
    {
        m |= (value);
    }

    layerMasks[layer] = m;
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
