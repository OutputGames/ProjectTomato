#ifndef UI_H
#define UI_H

#include "utils.hpp"
#include "Obj/obj.hpp"
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

        glm::vec2 getMin();
        glm::vec2 getMax();

        void CopyMinMax(glm::vec2 min, glm::vec2 max);

        bool isPointInRect(glm::vec2 p);
        bool isLineOnRect(glm::vec2 start, glm::vec2 end);
        bool isRectColliding(Rect r);
    };


    struct SpriteObject : obj::Object
    {
        render::Texture* mainTexture = nullptr;
        render::Material* material;
        render::Color mainColor;
        render::Mesh* spriteMesh = GetPrimitive(prim::Quad);
        bool isUI = true;
        bool useAlpha = true;
        bool copyAlpha = true;
        int layer = 0;

        SpriteObject();
        SpriteObject(string path);
        void Update() override;

        string GetDefaultName() override
        {
            return "Sprite";
        }
    };

    struct ButtonObject : obj::Object
    {
        void Start() override;
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
        SpriteObject* sprite;

        render::Texture* origTexture;
    };

    struct TextObject : SpriteObject
    {
        string text;
        render::Font* font;
        float size = 48;
        float spacing = FLT_MAX;

        TextObject();
        void Start() override;

        void Update() override;
        string GetDefaultName() override { return "Text"; }
    };

}

#endif
