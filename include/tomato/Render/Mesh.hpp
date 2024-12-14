#ifndef MESH_H
#define MESH_H

#include "utils.hpp" 
#include "tomato/Render/Vertex.hpp"
#include "tomato/Render/Material.hpp"
#include "tomato/Render/Material.hpp"


namespace tmt::render {

struct Mesh
{
    bgfx::IndexBufferHandle ibh;
    bgfx::VertexBufferHandle vbh;
    std::vector<bgfx::DynamicVertexBufferHandle> vertexBuffers;
    bgfx::DynamicIndexBufferHandle indexBuffer;
    size_t vertexCount, indexCount;
    Vertex *vertices;
    u16 *indices;

    void draw(glm::mat4 t, Material *material);
};

}

#endif