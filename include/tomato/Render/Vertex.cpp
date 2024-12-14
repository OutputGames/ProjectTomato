#include "Vertex.hpp" 
#include "globals.cpp" 

bgfx::VertexLayout tmt::render::Vertex::getVertexLayout()
{
    var layout = bgfx::VertexLayout();
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    return layout;
}

