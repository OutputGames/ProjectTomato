
#include "engine.hpp" 
#include "globals.hpp" 

tmt::engine::EngineInfo *tmt::engine::init()
{

    var resourceManager = new fs::ResourceManager();

    var rendererInfo = render::init();

    var engineInfo = new EngineInfo();
    engineInfo->renderer = rendererInfo;

    obj::init();
    audio::init();

    return engineInfo;
}

void tmt::engine::update()
{
#ifdef DEBUG
    debug::DebugUi::Update();
#endif


    obj::update();
    render::update();
    audio::update();

    double xpos = 0;
    double ypos = 0;

    glfwGetCursorPos(renderer->window, &xpos, &ypos);

    var p = glm::vec2{xpos, ypos};

    mousedelta = mousep - p;
    mousep = p;

    deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();
}

void tmt::engine::shutdown()
{ render::shutdown(); }
