#ifndef SUBSHADER_H
#define SUBSHADER_H

#include "utils.hpp" 
#include "tomato/Render/ShaderUniform.hpp"
#include "tomato/Render/ShaderUniform.hpp"


namespace tmt::render {

struct SubShader
{
    enum ShaderType
    {
        Vertex = 0,
        Fragment,
        Compute
    };

    bgfx::ShaderHandle handle;
    std::vector<ShaderUniform *> uniforms;

    SubShader(string name, ShaderType type);

    ShaderUniform *GetUniform(string name);
};

}

#endif