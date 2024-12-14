#include "ComputeShader.hpp" 
#include "globals.cpp" 

tmt::render::ComputeShader::ComputeShader(SubShader *shader)
{
    program = createProgram(shader->handle, true);
    internalShader = shader;
}

void tmt::render::ComputeShader::SetUniform(string name, bgfx::UniformType::Enum type, const void *data)
{
    var uni = createUniform(name.c_str(), type);

    setUniform(uni, data);

    destroy(uni);
}

void tmt::render::ComputeShader::SetMat4(string name, glm::mat4 m)
{
    // internalShader->GetUniform()
    for (int i = 0; i < 4; ++i)
    {
        SetUniform(name + "[" + std::to_string(i) + "]", bgfx::UniformType::Vec4, math::vec4toArray(m[0]));
    }
}

void tmt::render::ComputeShader::Run(int vid, glm::vec3 groups)
{
    for (var uni : internalShader->uniforms)
    {
        uni->Use();
    }

    dispatch(vid, program, groups.x, groups.y, groups.z);
}

