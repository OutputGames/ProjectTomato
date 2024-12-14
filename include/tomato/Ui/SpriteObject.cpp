#include "SpriteObject.hpp" 
#include "globals.cpp" 

void tmt::ui::SpriteObject::Update()
{
    var tex = material->GetUniform("s_texColor");
    var color = material->GetUniform("u_color");

    color->v4 = mainColor.getData();
    tex->tex = mainTexture;

    material->state.write = BGFX_STATE_WRITE_RGB;
    material->state.depth = render::MaterialState::Always;

    var drawCall = render::DrawCall();

    drawCall.mesh = GetPrimitive(prim::Quad);
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = render::MaterialState::OrthoProj;

    var transform = GetTransform();

    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            drawCall.transformMatrix[x][y] = transform[x][y];
        }
    }

    drawCall.program = material->shader;
    if (material->overrides.size() > 0)
    {
        drawCall.overrides = material->overrides.data();
        drawCall.overrideCt = material->overrides.size();
    }
    else
        drawCall.overrides = nullptr;

    pushDrawCall(drawCall);

    Object::Update();
}

