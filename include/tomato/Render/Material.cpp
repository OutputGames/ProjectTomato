#include "Material.hpp" 
#include "globals.cpp" 

tmt::render::MaterialOverride *tmt::render::Material::GetUniform(string name)
{
    for (auto override : overrides)
        if (override->name == name)
            return override;

    return nullptr;
}

u64 tmt::render::Material::GetMaterialState()
{
    u64 v = state.cull;
    v |= state.depth;
    v |= BGFX_STATE_WRITE_MASK;

    return v;
}

tmt::render::Material::Material(Shader *shader)
{
    Reload(shader);
}

void tmt::render::Material::Reload(Shader *shader)
{
    if (shader)
    {
        this->shader = shader;
        for (auto sub_shader : shader->subShaders)
        {
            for (auto uniform : sub_shader->uniforms)
            {
                var ovr = new MaterialOverride();
                ovr->name = uniform->name;

                overrides.push_back(ovr);
            }
        }
    }
}

