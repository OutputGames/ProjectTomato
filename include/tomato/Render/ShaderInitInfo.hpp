#ifndef SHADERINITINFO_H
#define SHADERINITINFO_H

#include "utils.hpp" 
#include "tomato/Render/SubShader.hpp"


namespace tmt::render {

struct ShaderInitInfo
{
    SubShader *vertexProgram, *fragmentProgram;
};

}

#endif