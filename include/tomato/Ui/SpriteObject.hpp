#ifndef SPRITEOBJECT_H
#define SPRITEOBJECT_H

#include "utils.hpp" 
#include "tomato/Render/Material.hpp"
#include "tomato/Render/Color.hpp"
#include "tomato/Obj/Object.hpp"
#include "tomato/Render/Texture.hpp"


namespace tmt::ui {

struct SpriteObject : obj::Object
{
    render::Texture *mainTexture;
    render::Material *material;
    render::Color mainColor;

    void Update() override;

    string GetDefaultName() override
    {
        return "Sprite";
    }
};

}

#endif