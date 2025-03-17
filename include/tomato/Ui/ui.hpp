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


    enum Anchor
    {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    struct SpriteObject : obj::Object
    {
        render::Texture* mainTexture = nullptr;
        render::Material* material;
        render::Color mainColor = render::Color::White;
        render::Mesh* spriteMesh = GetPrimitive(prim::Quad);
        bool isUI = true;
        bool useAlpha = true;
        bool copyAlpha = true;
        int layer = 0;

        Anchor anchor = Center;

        void Start() override;
        SpriteObject();
        SpriteObject(string path);
        void Update() override;

        ButtonObject* MakeButton();
        ButtonObject* GetButton();

        string GetDefaultName() override
        {
            return "Sprite";
        }

    private:
        ButtonObject* button;
    };

    struct ButtonObject : obj::Object
    {
        SpriteObject* sprite;

        void Start() override;
        void Update() override;

        void SetParent(Object* o);

        string GetDefaultName() override
        {
            return "Button";
        }

        int AddHoverEvent(std::function<void()> f);
        int AddClickEvent(std::function<void()> f);

    private:
        std::vector<std::function<void()>> hovers, clicks;

        bool hoverLast = false, clickLast = false;

        render::Texture* origTexture;
    };

    struct TextObject : SpriteObject
    {

        enum HTextAlign
        {
            Left,
            HCenter,
            Right
        };

        enum VTextAlign
        {
            Top,
            VCenter,
            Bottom
        };

        HTextAlign HorizontalAlign = HCenter;
        VTextAlign VerticalAlign = VCenter;

        string text;
        render::Font* font;
        float size = 48;
        float spacing = FLT_MAX;

        TextObject();
        void Start() override;

        void Update() override;
        string GetDefaultName() override { return "Text"; }

    private:
        SpriteObject* c = nullptr;
    };

    struct TextButtonObject : obj::Object
    {
        Rect rect;

        ButtonObject* button;
        TextObject* text;
        SpriteObject* sprite;

        TextButtonObject(Rect r, render::Font* font);

        void Update() override;
    };

}

#endif
