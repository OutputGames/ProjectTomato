#ifndef ENGINE_H
#define ENGINE_H

#include "utils.hpp" 
#include "tomato/Engine/EngineInfo.hpp"


namespace tmt::engine {

EngineInfo *init();

void update();
;

}

#endif