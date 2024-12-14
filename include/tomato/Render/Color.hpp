#ifndef COLOR_H
#define COLOR_H

#include "utils.hpp" 



namespace tmt::render {

struct Color
{
    float r = 1, g = 1, b = 1, a = 1;

    Color(float r = 1, float g = 1, float b = 1, float a = 1)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    glm::vec4 getData() const
    {
        return glm::vec4{r, g, b, a};
    }

    void darken(float amt)
    {
        r += amt;
        g += amt;
        b += amt;
    }

    u32 getHex() const
    {
        // Clamp values between 0 and 255 after scaling
        uint32_t red = static_cast<uint32_t>(r * 255) & 0xFF;
        uint32_t green = static_cast<uint32_t>(g * 255) & 0xFF;
        uint32_t blue = static_cast<uint32_t>(b * 255) & 0xFF;
        uint32_t alpha = static_cast<uint32_t>(a * 255) & 0xFF;

        // Combine into a single 32-bit integer in RGBA order
        return (alpha << 24) | (blue << 16) | (green << 8) | red;
    }

    static Color White, Red, Blue, Green;
};

}

#endif