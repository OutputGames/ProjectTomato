#ifndef ENGINE_H
#define ENGINE_H

#include "utils.hpp"


namespace tmt::render
{
    struct RendererInfo;
}


namespace tmt::engine
{
    struct Application;

    struct EngineInfo;

    struct EngineInfo
    {
        render::RendererInfo* renderer;
        Application* app;
    };

    EngineInfo* init(Application* app, glm::vec2 w);

    void update();

    void shutdown();

    struct Application
    {
        string name;
        bool is2D;
        EngineInfo* info;

        Application(string name, int width, int height, bool is2D);
    };

}

#endif
