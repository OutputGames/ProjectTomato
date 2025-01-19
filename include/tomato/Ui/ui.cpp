#include "ui.hpp"
#include "globals.hpp"

using namespace tmt::ui;

glm::vec2 Rect::getMin()
{
    return glm::vec2(x + (width / 2), y + (height / 2));
}

glm::vec2 Rect::getMax() { return glm::vec2(x - (width / 2), y - (height / 2)); }

void Rect::CopyMinMax(glm::vec2 min, glm::vec2 max)
{
    x = glm::lerp(min.x, max.x, 0.5f);
    y = glm::lerp(min.y, max.y, 0.5f);

    width = (max.x - x) * 2;
    height = (max.y - y) * 2;
}

int orientation(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r)
{
    float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0.0f)
        return 0; // Collinear
    return (val > 0.0f) ? 1 : 2; // Clockwise or Counterclockwise
}

bool onSegment(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r)
{
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) &&
        q.y >= std::min(p.y, r.y))
        return true;
    return false;
}


bool Rect::isPointInRect(glm::vec2 p)
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

bool checkLineLine(glm::vec2 start1, glm::vec2 end1, glm::vec2 start2, glm::vec2 end2)
{
    // Find the four orientations needed for the general and special cases
    int o1 = orientation(start1, end1, start2);
    int o2 = orientation(start1, end1, end2);
    int o3 = orientation(start2, end2, start1);
    int o4 = orientation(start2, end2, end1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special cases
    // start1, end1, and start2 are collinear and start2 lies on segment start1-end1
    if (o1 == 0 && onSegment(start1, start2, end1))
        return true;

    // start1, end1, and end2 are collinear and end2 lies on segment start1-end1
    if (o2 == 0 && onSegment(start1, end2, end1))
        return true;

    // start2, end2, and start1 are collinear and start1 lies on segment start2-end2
    if (o3 == 0 && onSegment(start2, start1, end2))
        return true;

    // start2, end2, and end1 are collinear and end1 lies on segment start2-end2
    if (o4 == 0 && onSegment(start2, end1, end2))
        return true;

    return false; // Doesn't fall in any of the above cases
}

bool Rect::isLineOnRect(glm::vec2 s, glm::vec2 e)
{
    glm::vec2 min = getMin();
    glm::vec2 max = getMax();

    glm::vec2 corners[4] = {
        {min.x, min.y}, // Bottom-left
        {max.x, min.y}, // Bottom-right
        {max.x, max.y}, // Top-right
        {min.x, max.y} // Top-left
    };

    // Check if the line intersects any of the rectangle's edges
    return checkLineLine(s, e, corners[0], corners[1]) || // Bottom edge
        checkLineLine(s, e, corners[1], corners[2]) || // Right edge
        checkLineLine(s, e, corners[2], corners[3]) || // Top edge
        checkLineLine(s, e, corners[3], corners[0]); // Left edge
}


bool Rect::isRectColliding(Rect r)
{
    var aMax = getMax();
    var aMin = getMin();

    var bMax = r.getMax();
    var bMin = r.getMin();

    if (aMax.x > bMin.x || aMin.x < bMax.x)
        return false;
    if (aMax.y > bMin.y || aMin.y < bMax.y)
        return false;

    return true;
}

SpriteObject::SpriteObject()
{
    var initInfo = render::ShaderInitInfo{
        render::SubShader::CreateSubShader("sprite/vert", render::SubShader::Vertex),
        render::SubShader::CreateSubShader("sprite/frag", render::SubShader::Fragment),
    };

    material = new render::Material(render::Shader::CreateShader(initInfo));

    material->state.SetWrite(BGFX_STATE_WRITE_RGB);
    material->state.depth = render::MaterialState::LessEqual;

    mainTexture = fs::ResourceManager::pInstance->loaded_textures["White"];
}

void SpriteObject::Update()
{
    var tex = material->GetUniform("s_texColor", true);
    var color = material->GetUniform("u_color", true);

    color->v4 = mainColor.getData();
    tex->tex = mainTexture;

    //material->state.write = BGFX_STATE_WRITE_RGB;
    //material->state.depth = render::MaterialState::Always;

    var drawCall = render::DrawCall();

    drawCall.layer = layer;
    drawCall.mesh = spriteMesh;
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = render::MaterialState::OrthoProj;

    var og_pos = position;

    if (!isUI)
    {

        position = GetGlobalPosition() - mainCameraObject->position;
        //position.x -= renderer->windowWidth / 2.0f;
        //position.y += renderer->windowHeight / 2.0f;

    }
    position -= glm::vec3(scale.x / 2, scale.y / 2, 0);

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

void ButtonObject::Update()
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

int ButtonObject::AddHoverEvent(std::function<void()> f)

{

    hovers.push_back(f);

    return hovers.size();
}

int ButtonObject::AddClickEvent(std::function<void()> f)

{

    clicks.push_back(f);

    return clicks.size();
}
