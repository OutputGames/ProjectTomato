#include "debug.h"

#include "debug.h"

#include <cassert>
#include <cstddef>
#include <iostream>

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"

#include "engine.hpp"

#ifdef _WIN32 
#include "renderdoc_app.h"
#include <windows.h>

RENDERDOC_API_1_1_2* rdoc_api = NULL;
tmglDebugRenderer* tmglDebugRenderer::renderer = nullptr;

void tmDebug::rdoc_init()
{
    // At init, on windows

    HMODULE mod = LoadLibraryA(RENDERDOC_HOME "/renderdoc.dll");

    if (mod)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        assert(ret == 1);
    } else
    {
	    std::cerr << "RenderDOC api not found." << std::endl;
    }
}

void tmDebug::rdoc_beginframe()
{
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
}

void tmDebug::rdoc_endframe()
{
    if (rdoc_api)
    {
	    rdoc_api->EndFrameCapture(NULL, NULL);
        rdoc_api->LaunchReplayUI(1, NULL);
    }
}

void tmDebug::tmgl_initdbg()
{
	tmglDebugRenderer::renderer = new tmglDebugRenderer;
    dd::initialize(tmglDebugRenderer::renderer);
}

void tmDebug::tmgl_endframe()
{
    dd::flush();
}

tmglDebugRenderer::tmglDebugRenderer()
{

    const char* linePointVertShaderSrc = "\n"
        "#version 150\n"
        "\n"
        "in vec3 in_Position;\n"
        "in vec4 in_ColorPointSize;\n"
        "\n"
        "out vec4 v_Color;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
		"uniform float pointSize;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position  = projection * view * vec4(in_Position, 1.0);\n"
        "    gl_PointSize = in_ColorPointSize.w*pointSize;\n"
        "    v_Color      = vec4(in_ColorPointSize.xyz, 1.0);\n"
        "}\n";
    const char* linePointFragShaderSrc = "\n"
        "#version 150\n"
        "\n"
        "in  vec4 v_Color;\n"
        "out vec4 out_FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    out_FragColor = v_Color;\n"
        "}\n";

    linePointShader = new tmShader(linePointVertShaderSrc, linePointFragShaderSrc, false);


    {
        glGenVertexArrays(1, &linePointVAO);
        glGenBuffers(1, &linePointVBO);

        glBindVertexArray(linePointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

        // RenderInterface will never be called with a batch larger than
        // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
        glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);

        // Set the vertex format expected by 3D points and lines:
        std::size_t offset = 0;

        glEnableVertexAttribArray(0); // in_Position (vec3)
        glVertexAttribPointer(
            /* index     = */ 0,
            /* size      = */ 3,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void*>(offset));
        offset += sizeof(float) * 3;

        glEnableVertexAttribArray(1); // in_ColorPointSize (vec4)
        glVertexAttribPointer(
            /* index     = */ 1,
            /* size      = */ 4,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void*>(offset));


        // VAOs can be a pain in the neck if left enabled...
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

#endif

void tmglDebugRenderer::drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled)
{
    linePointShader->setFloat("pointSize", pointsize);
    tmeGetCore()->renderMgr->InsertCallback( [&points, &count, &depthEnabled, this](tmBaseCamera* cam)
    {
            assert(points != nullptr);
            assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

            glBindVertexArray(linePointVAO);

            linePointShader->setMat4("projection", cam->GetProjectionMatrix());
            linePointShader->setMat4("view", cam->GetViewMatrix());

            if (depthEnabled)
            {
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }

            // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
            glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), points);

            // Issue the draw call:
            glDrawArrays(GL_POINTS, 0, count);

            glUseProgram(0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
    } );
}

void tmglDebugRenderer::drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled)
{
    linePointShader->setFloat("pointSize", pointsize);
    tmeGetCore()->renderMgr->InsertCallback([lines, count, depthEnabled, this](tmBaseCamera* cam)
        {
            assert(lines != nullptr);
            assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

            glBindVertexArray(linePointVAO);

            linePointShader->setMat4("projection", cam->GetProjectionMatrix());
            linePointShader->setMat4("view", cam->GetViewMatrix());

            if (depthEnabled)
            {
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }

            // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
            glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), lines);

            // Issue the draw call:
            glDrawArrays(GL_LINES, 0, count);

            glUseProgram(0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        });
}


void tmglDebugRenderer::box(vec3 origin, ddVec3_In color, vec3 size)
{
    dd::box(glmtodd(origin), color, size.x, size.y, size.z);
}

void tmglDebugRenderer::sphere(vec3 origin, ddVec3_In color, float radius)
{
    dd::sphere(glmtoddin(origin), color, radius);
}

void tmglDebugRenderer::line(vec3 start, vec3 end, ddVec3_In color)
{
    dd::line(glmtoddin(start), glmtoddin(end), color);
}

void tmglDebugRenderer::frustum(mat4 clipMatrix, ddVec3_In color)
{
    dd::frustum(glm::value_ptr(clipMatrix), color);
}

void tmglDebugRenderer::grid(float size, vec3 position, float squareSize, ddVec3_In color)
{
    if (squareSize <= EPSILON)
        return;
    ddVec3 from, to;
    for (float i = -size; i <= size; i += squareSize)
    {
        // Horizontal line (along the X)
        dd::vecSet(from, -size+position.x, position.y, i+position.z);
        dd::vecSet(to, size+position.x, position.y, i+position.z);
        dd::line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) from, to, color);

        // Vertical line (along the Z)
        dd::vecSet(from, i+position.x, position.y, -size+position.z);
        dd::vecSet(to, i+position.x, position.y, size+position.z);
        dd::line(DD_EXPLICIT_CONTEXT_ONLY(ctx, ) from, to, color);
    }
}

void tmglDebugRenderer::cone(vec3 origin, vec3 dir, float min, float max, ddVec3_In color)
{
    dd::cone(glmtodd(origin), glmtoddin(dir), color, min, max);
}
