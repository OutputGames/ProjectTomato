#include "ShaderUniform.hpp" 
#include "globals.cpp" 

void tmt::render::ShaderUniform::Use()
{
    switch (type)
    {
    case bgfx::UniformType::Sampler:
        if (tex)
        {
            setTexture(0, handle, tex->handle);
        }
        break;
    case bgfx::UniformType::End:
        break;
    case bgfx::UniformType::Vec4:
        setUniform(handle, math::vec4toArray(v4));
        break;
    case bgfx::UniformType::Mat3:
        setUniform(handle, math::mat3ToArray(m3));
        break;
    case bgfx::UniformType::Mat4: {
        float m[4][4];
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                m[x][y] = m4[x][y];
            }
        }

        setUniform(handle, m);
    }
    break;
    case bgfx::UniformType::Count:
        break;
    default:
        break;
    }
}

