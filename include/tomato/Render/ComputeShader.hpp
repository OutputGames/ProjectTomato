#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

#include "utils.hpp" 
#include "tomato/Render/SubShader.hpp"


namespace tmt::render {

struct ComputeShader
{
    bgfx::ProgramHandle program;
    SubShader *internalShader;

    ComputeShader(SubShader *shader);

    void SetUniform(string name, bgfx::UniformType::Enum type, const void *data);

    void SetMat4(string name, glm::mat4 m);

    void Run(int viewId, glm::vec3 groups = {1, 1, 1});
};

}

#endif