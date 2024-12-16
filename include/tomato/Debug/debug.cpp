#include "debug.hpp" 
#include "globals.hpp" 

void tmt::debug::Gizmos::DrawLine(glm::vec3 start, glm::vec3 end)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Line, start, end, 0, clr});
}

void tmt::debug::Gizmos::DrawSphere(glm::vec3 position, float radius)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Sphere, position, glm::vec3{0}, radius, clr});
}

void tmt::debug::Gizmos::_DrawText(glm::vec2 pos, string text)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Text, glm::vec3(pos,0), glm::vec3{0}, 0, clr, text});
}
