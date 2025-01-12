#include "ui.hpp"
#include "globals.hpp"

bool tmt::ui::Rect::isPointInRect(glm::vec2 p)
{
    if (p.x >= x && p.x <= x + width)
    {
        if (p.y >= y && p.y <= y + height)
        {
            return true;
        }
    }
    return false;
}

tmt::ui::SpriteObject::SpriteObject()
{
    var initInfo = render::ShaderInitInfo{
        render::SubShader::CreateSubShader("sprite/vert", render::SubShader::Vertex),
        render::SubShader::CreateSubShader("sprite/frag", render::SubShader::Fragment),
    };

    material = new render::Material(render::Shader::CreateShader(initInfo));
    mainTexture = fs::ResourceManager::pInstance->loaded_textures["White"];
}

void tmt::ui::SpriteObject::Update()
{
    var tex = material->GetUniform("s_texColor", true);
    var color = material->GetUniform("u_color", true);

    color->v4 = mainColor.getData();
    tex->tex = mainTexture;

    material->state.write = BGFX_STATE_WRITE_RGB;
    material->state.depth = render::MaterialState::Always;

    var drawCall = render::DrawCall();

    drawCall.mesh = GetPrimitive(prim::Quad);
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = render::MaterialState::OrthoProj;

    var og_pos = position;

    if (!isUI)
    {

        position = GetGlobalPosition() - mainCameraObject->position;
        position.x -= renderer->windowWidth / 2.0f;
        position.y += renderer->windowHeight / 2.0f;

        position -= glm::vec3(scale.x / 2, scale.y / 2, 0);
    }

    var transform = GetLocalTransform();

    if (isUI)
    {
        transform = GetTransform();
    }

    position = og_pos;

    drawCall.transformMatrix = transform;

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

void tmt::ui::ButtonObject::Update()
{
    var pos = input::Mouse::GetMousePosition();

    var gpos = GetGlobalPosition();
    var gscl = GetGlobalScale();
    var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

    bool hover = rect.isPointInRect(pos);
    bool click = false;

    // mama a typecheck behind u
    var sprite = dynamic_cast<SpriteObject*>(parent);

    float darkenAmt = 0.25;

    if (hover)
    {
        if (!hoverLast)
        {
            if (sprite)
            {
                sprite->mainColor.darken(-darkenAmt);
            }
        }
        for (const auto& function : hovers)
        {
            function();
        }

        if (input::Mouse::GetMouseButton(input::Mouse::Left) == input::Mouse::Press)
        {
            click = true;
            if (!clickLast)
            {
                for (const auto& function : clicks)
                {
                    function();
                }
            }
        }
    }
    else
    {
        if (hoverLast)
        {
            if (sprite)
            {
                sprite->mainColor.darken(darkenAmt);
            }
        }
    }

    hoverLast = hover;
    clickLast = click;
}

int tmt::ui::ButtonObject::AddHoverEvent(std::function<void()> f)

{

    hovers.push_back(f);

    return hovers.size();
}

int tmt::ui::ButtonObject::AddClickEvent(std::function<void()> f)

{

    clicks.push_back(f);

    return clicks.size();
}
