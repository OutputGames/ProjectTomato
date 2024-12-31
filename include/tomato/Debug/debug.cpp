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

void tmt::debug::Gizmos::DrawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    var clr = color;

    var triangle = new DebugTriangle{v1, v2, v3};

    debugCalls.push_back(DebugCall{Triangle, glm::vec3(0), glm::vec3{0}, 1, clr, "", matrix, triangle});
    matrix = glm::mat4(1.0);
}

void tmt::debug::Gizmos::DrawTriangles(DebugTriangle* triangles, int triCt)
{
    var clr = color;

    debugCalls.push_back(DebugCall{TriangleList, glm::vec3(0), glm::vec3{0}, static_cast<float>(triCt), clr, "", matrix,
                                   triangles});
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
