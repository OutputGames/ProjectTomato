#ifndef MATERIAL_H
#define MATERIAL_H

#include "utils.hpp" 
#include "tomato/Render/Shader.hpp"
#include "tomato/Render/MaterialOverride.hpp"
#include "tomato/Render/MaterialState.hpp"
#include "tomato/Render/MaterialOverride.hpp"


namespace tmt::render {

struct Material
{
    MaterialState state;
    Shader *shader;
    std::vector<MaterialOverride *> overrides;

    MaterialOverride *GetUniform(string name);
    u64 GetMaterialState();

    Material(Shader *shader = nullptr);

    void Reload(Shader *shader);
};

}

#endif