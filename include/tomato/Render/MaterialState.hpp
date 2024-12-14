#ifndef MATERIALSTATE_H
#define MATERIALSTATE_H

#include "utils.hpp" 



namespace tmt::render {

struct MaterialState
{
    enum DepthTest
    {
        Less = BGFX_STATE_DEPTH_TEST_LESS,
        LessEqual = BGFX_STATE_DEPTH_TEST_LEQUAL,
        Equal = BGFX_STATE_DEPTH_TEST_EQUAL,
        GreaterEqual = BGFX_STATE_DEPTH_TEST_GEQUAL,
        Greater = BGFX_STATE_DEPTH_TEST_GREATER,
        NotEqual = BGFX_STATE_DEPTH_TEST_NOTEQUAL,
        Never = BGFX_STATE_DEPTH_TEST_NEVER,
        Always = BGFX_STATE_DEPTH_TEST_ALWAYS,
    } depth = Less;

    enum CullMode
    {
        Clockwise = BGFX_STATE_CULL_CW,
        Counterclockwise = BGFX_STATE_CULL_CCW
    } cull = Counterclockwise;

    enum WriteMode
    {
        Red = BGFX_STATE_WRITE_R,
        Green = BGFX_STATE_WRITE_G,
        Blue = BGFX_STATE_WRITE_B,
        Alpha = BGFX_STATE_WRITE_A,
        Depth = BGFX_STATE_WRITE_Z,
        All = BGFX_STATE_WRITE_MASK
    };

    u64 write = All;

    enum MatrixMode
    {
        ViewProj,
        View,
        Proj,
        ViewOrthoProj,
        OrthoProj,
        None
    } matrixMode = ViewProj;
};

}

#endif