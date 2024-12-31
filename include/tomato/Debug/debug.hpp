#ifndef DEBUG_H
#define DEBUG_H

#include "utils.hpp"
#include "tomato/Render/render.hpp"


namespace tmt::debug
{

    enum DebugCallType;
    struct DebugCall;
    struct Gizmos;

    enum DebugCallType
    {
        Line,
        Sphere,
        Text,
        Box,
        Triangle,
        TriangleList
    };

    struct DebugTriangle
    {
        glm::vec3 p1, p2, p3;

        glm::vec3& operator[](std::size_t index)
        {
            switch (index)
            {
                case 0:
                    return p1;
                case 1:
                    return p2;
                case 2:
                    return p3;
                default:
                    throw std::out_of_range("Index out of range");
            }
        }

        const glm::vec3& operator[](std::size_t index) const
        {
            switch (index)
            {
                case 0:
                    return p1;
                case 1:
                    return p2;
                case 2:
                    return p3;
                default:
                    throw std::out_of_range("Index out of range");
            }
        }
    };


    struct DebugCall
    {
        DebugCallType type;
        glm::vec3 origin, direction;
        float radius;
        render::Color color;
        string text = "";
        glm::mat4 matrix;
        DebugTriangle* triangles;
    };

    struct Gizmos
    {

        inline static render::Color color = render::Color::White;
        inline static auto matrix = glm::mat4(1.0);

        static void DrawLine(glm::vec3 start, glm::vec3 end);
        static void DrawBox(glm::vec3 pos, glm::vec3 size);
        static void DrawSphere(glm::vec3 position, float radius);
        static void _DrawText(glm::vec2 pos, string text);
        static void DrawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
        static void DrawTriangles(DebugTriangle* triangles, int triangleCount);
    };
    ;

    struct DebugUi
    {
        static void AddImguiEvent(std::function<void()> func);
        static void AddRecurringDbgEvent(std::function<void()> func);

        static void Update();
    };

}

#endif
