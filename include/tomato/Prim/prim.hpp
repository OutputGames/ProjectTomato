#ifndef PRIM_H
#define PRIM_H

#include "utils.hpp"


namespace tmt::render
{
    struct Mesh;
}


namespace tmt::prim
{

    enum PrimitiveType;

    enum PrimitiveType
    {
        Quad,
        Cube,
        Sphere,
        Cylinder,
    };

    render::Mesh* GetPrimitive(PrimitiveType type);
    ;

}

#endif
