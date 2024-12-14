#ifndef TEXTURE_H
#define TEXTURE_H

#include "utils.hpp" 



namespace tmt::render {

struct Texture
{
    bgfx::TextureHandle handle;
    bgfx::TextureFormat::Enum format;

    int width, height;

    Texture(string path);
    Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory *mem = nullptr);
};

}

#endif