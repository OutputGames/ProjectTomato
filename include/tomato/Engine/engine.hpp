#ifndef ENGINE_H
#define ENGINE_H

#include "utils.hpp" 



namespace tmt::render {
 struct RendererInfo;
 }


namespace tmt::engine {

struct EngineInfo;

struct EngineInfo
{
    render::RendererInfo *renderer;
};

EngineInfo *init();

void update();

    void shutdown();
;

}

#endif