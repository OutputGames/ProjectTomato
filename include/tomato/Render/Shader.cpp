#include "Shader.hpp" 
#include "globals.cpp" 

tmt::render::Shader::Shader(ShaderInitInfo info)
{
    program = createProgram(info.vertexProgram->handle, info.fragmentProgram->handle, true);

    subShaders.push_back(info.vertexProgram);
    subShaders.push_back(info.fragmentProgram);
}

void tmt::render::Shader::Push(int viewId, MaterialOverride **overrides, size_t oc)
{
    std::map<std::string, MaterialOverride> m_overrides;

    if (overrides != nullptr)
    {
        for (int i = 0; i < oc; ++i)
        {
            var name = overrides[i]->name;

            var pair = std::make_pair<std::string, MaterialOverride>(name, *overrides[i]);
            m_overrides.insert(pair);
        }
    }

    for (var shader : subShaders)
    {
        for (var uni : shader->uniforms)
        {
            if (overrides != nullptr)
            {
                if (m_overrides.contains(uni->name))
                {
                    var ovr = m_overrides.find(uni->name)->second;
                    uni->v4 = ovr.v4;
                    uni->m3 = ovr.m3;
                    uni->m4 = ovr.m4;
                    uni->tex = ovr.tex;
                }
            }

            uni->Use();
        }
    }

    submit(viewId, program);
}

