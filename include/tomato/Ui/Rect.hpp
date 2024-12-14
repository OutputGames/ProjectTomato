#ifndef RECT_H
#define RECT_H

#include "utils.hpp" 



namespace tmt::ui {

struct Rect
{
    float x, y;
    float width, height;

    bool isPointInRect(glm::vec2 p);
};

}

#endif