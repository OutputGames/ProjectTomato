#ifndef SHADERUNIFORM_H
#define SHADERUNIFORM_H

#include "utils.hpp" 
#include "tomato/Render/Texture.hpp"


namespace tmt::render {

struct ShaderUniform
{
    bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
    string name;
    bgfx::UniformType::Enum type;

    glm::vec4 v4 = glm::vec4(0);
    glm::mat3 m3 = glm::mat3(1.0);
    glm::mat4 m4 = glm::mat4(1.0);
    Texture *tex = nullptr;

    void Use();
};

}

#endif