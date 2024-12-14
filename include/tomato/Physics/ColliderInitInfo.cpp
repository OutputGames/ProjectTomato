#include "ColliderInitInfo.hpp" 
#include "globals.cpp" 

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForSphere(float radius)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.s = Sphere;

    return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForCapsule(float radius, float height)
{
    var info = ColliderInitInfo();

    info.radius = radius;
    info.height = height;
    info.s = Capsule;

    return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForMesh(render::Mesh *mesh)
{
    var info = ColliderInitInfo();

    info.mesh = mesh;
    info.s = Mesh;

    return info;
}

