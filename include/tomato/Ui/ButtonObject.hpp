#ifndef BUTTONOBJECT_H
#define BUTTONOBJECT_H

#include "utils.hpp" 
#include "tomato/Obj/Object.hpp"


namespace tmt::ui {

struct ButtonObject : obj::Object
{
    void Update() override;

    string GetDefaultName() override
    {
        return "Button";
    }

    int AddHoverEvent(std::function<void()> f);
    int AddClickEvent(std::function<void()> f);

  private:
    std::vector<std::function<void()>> hovers, clicks;

    bool hoverLast, clickLast;
};

}

#endif