#ifndef UI_H
#define UI_H

#include "utils.hpp"
#include "tomato/Obj/obj.hpp"
#include "tomato/Obj/obj.hpp"
#include "tomato/Render/render.hpp"


namespace tmt::ui
{

    struct Rect;
    struct SpriteObject;
    struct ButtonObject;

    struct Rect
    {
        float x, y;
        float width, height;

        bool isPointInRect(glm::vec2 p);
    };

    struct SpriteObject : obj::Object
    {
        render::Texture* mainTexture;
        render::Material* material;
        render::Color mainColor;

        SpriteObject();
        void Update() override;

        string GetDefaultName() override
        {
            return "Sprite";
        }
    };

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
    ;

}

#endif
