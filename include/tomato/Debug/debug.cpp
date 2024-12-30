#include "debug.hpp"
#include "globals.hpp"

std::vector<std::function<void()>> recurringDbgs;

void tmt::debug::Gizmos::DrawLine(glm::vec3 start, glm::vec3 end)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Line, start, end, 0, clr, "", matrix});

    matrix = glm::mat4(1.0);
}

void tmt::debug::Gizmos::DrawBox(glm::vec3 pos, glm::vec3 size)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Box, pos, size, 0, clr, "", matrix});

    matrix = glm::mat4(1.0);
}

void tmt::debug::Gizmos::DrawSphere(glm::vec3 position, float radius)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Sphere, position, glm::vec3{0}, radius, clr, "", matrix});

    matrix = glm::mat4(1.0);
}

void tmt::debug::Gizmos::_DrawText(glm::vec2 pos, string text)
{
    var clr = color;

    debugCalls.push_back(DebugCall{Text, glm::vec3(pos, 0), glm::vec3{0}, 0, clr, text, matrix});
    matrix = glm::mat4(1.0);
}

void tmt::debug::DebugUi::AddImguiEvent(std::function<void()> func)
{
    debugFuncs.push_back(func);
}

void tmt::debug::DebugUi::AddRecurringDbgEvent(std::function<void()> func)
{
    recurringDbgs.push_back(func);
}

void tmt::debug::DebugUi::Update()
{
    for (auto recurring_dbg : recurringDbgs)
    {
        debugFuncs.push_back(recurring_dbg);
    }
}
