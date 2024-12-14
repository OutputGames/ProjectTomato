#ifndef SHADER_H
#define SHADER_H

#include "utils.hpp" 
#include "tomato/Render/MaterialOverride.hpp"
#include "tomato/Render/ShaderInitInfo.hpp"
#include "tomato/Render/SubShader.hpp"
#include "tomato/Render/MaterialOverride.hpp"
#include "tomato/Render/ShaderInitInfo.hpp"


namespace tmt::render {

struct Shader
{
    bgfx::ProgramHandle program;
    std::vector<SubShader *> subShaders;

    Shader(ShaderInitInfo info);

    void Push(int viewId = 0, MaterialOverride **overrides = nullptr, size_t overrideCount = 0);
};

}

#endif