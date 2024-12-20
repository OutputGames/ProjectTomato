#ifndef DEBUG_H
#define DEBUG_H

#include "utils.hpp" 
#include "tomato/Render/render.hpp"




namespace tmt::debug {

enum DebugCallType;
struct DebugCall;
struct Gizmos;

enum DebugCallType
{
    Line,
    Sphere,
    Text
};

struct DebugCall
{
    DebugCallType type;
    glm::vec3 origin, direction;
    float radius;
    render::Color color;
    string text = "";
};

struct Gizmos
{

    inline static render::Color color = render::Color::White;

    static void DrawLine(glm::vec3 start, glm::vec3 end);
    static void DrawSphere(glm::vec3 position, float radius);
    static void _DrawText(glm::vec2 pos,string text);
};
;

}

#endif