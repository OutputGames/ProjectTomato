#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include "utils.hpp" 
#include "tomato/Render/Texture.hpp"


namespace tmt::render {

struct RenderTexture
{
    bgfx::FrameBufferHandle handle;
    bgfx::ViewId vid = 1;
    Texture *realTexture;

    bgfx::TextureFormat::Enum format;

    RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 clearFlags);
};

}

#endif