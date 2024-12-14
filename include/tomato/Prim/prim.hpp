#ifndef PRIM_H
#define PRIM_H

#include "utils.hpp" 
#include "tomato/Render/Mesh.hpp"
#include "tomato/Prim/PrimitiveType.hpp"


namespace tmt::prim {

render::Mesh *GetPrimitive(PrimitiveType type);
;

}

#endif