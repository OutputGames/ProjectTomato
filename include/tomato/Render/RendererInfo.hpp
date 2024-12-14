#ifndef RENDERERINFO_H
#define RENDERERINFO_H

#include "utils.hpp" 



namespace tmt::render {

struct RendererInfo
{
    GLFWwindow *window;
    bgfx::ViewId clearView;
    int windowWidth, windowHeight;
};

}

#endif