#include "ButtonObject.hpp" 
#include "globals.cpp" 

void tmt::ui::ButtonObject::Update()
{
    var pos = input::Mouse::GetMousePosition();

    var gpos = GetGlobalPosition();
    var gscl = GetGlobalScale();
    var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

    bool hover = rect.isPointInRect(pos);
    bool click = false;

    // mama a typecheck behind u
    var sprite = dynamic_cast<SpriteObject *>(parent);

    float darkenAmt = 0.25;

    if (hover)
    {
        if (!hoverLast)
        {
            if (sprite)
            {
                sprite->mainColor.darken(-darkenAmt);
            }
        }
        for (const auto &function : hovers)
        {
            function();
        }

        if (input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == input::Mouse::Press)
        {
            click = true;
            if (!clickLast)
            {
                for (const auto &function : clicks)
                {
                    function();
                }
            }
        }
    }
    else
    {
        if (hoverLast)
        {
            if (sprite)
            {
                sprite->mainColor.darken(darkenAmt);
            }
        }
    }

    hoverLast = hover;
    clickLast = click;
}

int tmt::ui::ButtonObject::AddHoverEvent(std::function<void()> f)
{
    hovers.push_back(f);
    return hovers.size();
}

int tmt::ui::ButtonObject::AddClickEvent(std::function<void()> f)
{
    clicks.push_back(f);
    return clicks.size();
}

