#include "2d.hpp"

#include "globals.hpp"
#include "Time/time.hpp"
#include "box2d/box2d.h"

using namespace tmt::engine2D;

physics::PhysicsBody2D::PhysicsBody2D()
{
    collider = GetObjectFromType<PhysicsCollider2D>();
    
    mainScene->physicsWorld2D.
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
