#include "engine.hpp"
#include "globals.hpp"

tmt::engine::EngineInfo* tmt::engine::init(Application* app, glm::vec2 ws)
{
    application = app;
    var resourceManager = new fs::ResourceManager();

    var rendererInfo = render::init(ws.x, ws.y);

    input::init();

    var engineInfo = new EngineInfo();
    engineInfo->renderer = rendererInfo;
    engineInfo->app = app;

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
    input::Update();
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
{
    render::shutdown();
}

tmt::engine::Application::Application(string name, int width, int height, bool _2d)
{
    this->name = name;
    this->is2D = _2d;

    info = init(this, glm::vec2(width, height));
    info->app = this;
}
