#include "Texture.hpp" 
#include "globals.cpp" 

tmt::render::Texture::Texture(string path)
{
    int nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    uint64_t textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_POINT; // Adjust as needed
    bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA8;

    // Create the texture in bgfx, passing the image data directly
    handle = createTexture2D(static_cast<uint16_t>(width), static_cast<uint16_t>(height),
                             false, // no mip-maps
                             1,     // single layer
                             textureFormat, textureFlags,
                             bgfx::copy(data, width * height * nrChannels) // copies the image data
    );

    stbi_image_free(data);

    format = textureFormat;
}

tmt::render::Texture::Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory *mem)
{
    handle = createTexture2D(width, height, false, 1, tf, flags, mem);

    format = tf;
    this->width = width;
    this->height = height;
}

