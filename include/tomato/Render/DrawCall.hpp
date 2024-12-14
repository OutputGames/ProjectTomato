#ifndef DRAWCALL_H
#define DRAWCALL_H

#include "utils.hpp" 
#include "tomato/Render/Shader.hpp"
#include "tomato/Render/MaterialOverride.hpp"
#include "tomato/Render/Mesh.hpp"
#include "tomato/Render/MaterialState.hpp"


namespace tmt::render {

struct DrawCall
{
    u64 state;

    MaterialOverride **overrides;
    size_t overrideCt = 0;

    Mesh *mesh;
    float transformMatrix[4][4];
    Shader *program;
    MaterialState::MatrixMode matrixMode;
};

}

#endif