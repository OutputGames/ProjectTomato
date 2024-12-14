#ifndef RENDER_H
#define RENDER_H

#include "utils.hpp" 
#include "tomato/Render/Mesh.hpp"
#include "tomato/Render/RendererInfo.hpp"
#include "tomato/Render/Vertex.hpp"
#include "tomato/Render/DrawCall.hpp"


namespace tmt::render {

Mesh *createMesh(Vertex *data, u16 *indices, u32 vertSize, u32 triSize, bgfx::VertexLayout pcvDecl);

void pushDrawCall(DrawCall d);

RendererInfo *init();

void update();

void shutdown();
;

}

#endif