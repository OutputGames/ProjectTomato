#ifndef COLLIDERINITINFO_H
#define COLLIDERINITINFO_H

#include "utils.hpp" 
#include "tomato/Physics/CollisionShape.hpp"
#include "tomato/Render/Mesh.hpp"


namespace tmt::physics {

struct ColliderInitInfo
{
    CollisionShape s;
    glm::vec3 bounds;
    float radius;
    float height;
    render::Mesh *mesh;

    static ColliderInitInfo ForBox(glm::vec3 bounds);
    static ColliderInitInfo ForSphere(float radius);
    static ColliderInitInfo ForCapsule(float radius, float height);
    static ColliderInitInfo ForMesh(render::Mesh *mesh);
};

}

#endif