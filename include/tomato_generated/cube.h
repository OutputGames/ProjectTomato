#ifndef CUBE_MESH_H
#define CUBE_MESH_H

#include "vertex.h"

namespace cube_mesh
{
    static tmt::render::Vertex vertices_0[] = {
        {glm::vec3{-0.5, 0.5, 0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.625, 0.0}},
        {glm::vec3{-0.5, -0.5, -0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.375, 0.25}},
        {glm::vec3{-0.5, -0.5, 0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.375, 0.0}},
        {glm::vec3{-0.5, 0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.625, 0.25}},
        {glm::vec3{0.5, -0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.375, 0.5}},
        {glm::vec3{-0.5, -0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.375, 0.25}},
        {glm::vec3{0.5, 0.5, -0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.625, 0.5}},
        {glm::vec3{0.5, -0.5, 0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.375, 0.75}},
        {glm::vec3{0.5, -0.5, -0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.375, 0.5}},
        {glm::vec3{0.5, 0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.625, 0.75}},
        {glm::vec3{-0.5, -0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.375, 1.0}},
        {glm::vec3{0.5, -0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.375, 0.75}},
        {glm::vec3{0.5, -0.5, -0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.375, 0.5}},
        {glm::vec3{-0.5, -0.5, 0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.125, 0.75}},
        {glm::vec3{-0.5, -0.5, -0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.125, 0.5}},
        {glm::vec3{-0.5, 0.5, -0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.875, 0.5}},
        {glm::vec3{0.5, 0.5, 0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.625, 0.75}},
        {glm::vec3{0.5, 0.5, -0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.625, 0.5}},
        {glm::vec3{-0.5, 0.5, 0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.625, 0.0}},
        {glm::vec3{-0.5, 0.5, -0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.625, 0.25}},
        {glm::vec3{-0.5, -0.5, -0.5}, glm::vec3{-1.0, -0.0, -0.0}, glm::vec2{0.375, 0.25}},
        {glm::vec3{-0.5, 0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.625, 0.25}},
        {glm::vec3{0.5, 0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.625, 0.5}},
        {glm::vec3{0.5, -0.5, -0.5}, glm::vec3{-0.0, -0.0, -1.0}, glm::vec2{0.375, 0.5}},
        {glm::vec3{0.5, 0.5, -0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.625, 0.5}},
        {glm::vec3{0.5, 0.5, 0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.625, 0.75}},
        {glm::vec3{0.5, -0.5, 0.5}, glm::vec3{1.0, -0.0, -0.0}, glm::vec2{0.375, 0.75}},
        {glm::vec3{0.5, 0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.625, 0.75}},
        {glm::vec3{-0.5, 0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.625, 1.0}},
        {glm::vec3{-0.5, -0.5, 0.5}, glm::vec3{-0.0, -0.0, 1.0}, glm::vec2{0.375, 1.0}},
        {glm::vec3{0.5, -0.5, -0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.375, 0.5}},
        {glm::vec3{0.5, -0.5, 0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.375, 0.75}},
        {glm::vec3{-0.5, -0.5, 0.5}, glm::vec3{-0.0, -1.0, -0.0}, glm::vec2{0.125, 0.75}},
        {glm::vec3{-0.5, 0.5, -0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.875, 0.5}},
        {glm::vec3{-0.5, 0.5, 0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.875, 0.75}},
        {glm::vec3{0.5, 0.5, 0.5}, glm::vec3{-0.0, 1.0, -0.0}, glm::vec2{0.625, 0.75}},
    };

    static uint16_t indices_0[] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35,
    };

    static uint16_t vertexCount_0 = 36;
    static uint16_t indexCount_0 = 36;

};

#endif
