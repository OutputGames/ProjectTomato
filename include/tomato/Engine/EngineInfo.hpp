#ifndef ENGINEINFO_H
#define ENGINEINFO_H

#include "utils.hpp" 
#include "tomato/Render/RendererInfo.hpp"


namespace tmt::engine {

struct EngineInfo
{
    render::RendererInfo *renderer;
};

}

#endif