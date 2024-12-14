#include "Rect.hpp" 
#include "globals.cpp" 

bool tmt::ui::Rect::isPointInRect(glm::vec2 p)
{
    if (p.x >= x && p.x <= x + width)
    {
        if (p.y >= y && p.y <= y + height)
        {
            return true;
        }
    }
    return false;
}

