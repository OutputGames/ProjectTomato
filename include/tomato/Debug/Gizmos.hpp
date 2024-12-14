#ifndef GIZMOS_H
#define GIZMOS_H

#include "utils.hpp" 
#include "tomato/Render/Color.hpp"


namespace tmt::debug {

struct Gizmos
{
    enum DebugCallType
    {
        Line,
        Sphere
    };

    struct DebugCall
    {
        DebugCallType type;
        glm::vec3 origin, direction;
        float radius;
        render::Color color;
    };

    inline static render::Color color = render::Color::White;

    static void DrawLine(glm::vec3 start, glm::vec3 end);
    static void DrawSphere(glm::vec3 position, float radius);
};

}

#endif