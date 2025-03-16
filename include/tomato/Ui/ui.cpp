#include "ui.hpp"
#include "globals.hpp"

using namespace tmt::ui;


SpriteObject* MakeCorner(glm::vec2 pos)
{
    var sprite = new SpriteObject();
    sprite->scale = glm::vec3(10);
    sprite->position = glm::vec3(pos, 0);
    sprite->mainColor = tmt::render::Color::Blue;

    return sprite;
}

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
    var min = getMin();
    var max = getMax();

    bool r = p.x > max.x;
    bool l = p.x < min.x;
    bool t = p.y < min.y;
    bool b = p.y > max.y;

    if (r && l && t && b)
    {
        return true;
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

void SpriteObject::Start()
{
    Object::Start();

    /*
    if (name == "menu")
    {

        var gpos = GetGlobalPosition();
        var gscl = GetGlobalScale();
        var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

        var min = rect.getMin();
        var max = rect.getMax();

        // var corner1 = MakeCorner(glm::vec2(min.x, min.y));
        // var corner2 = MakeCorner(glm::vec2(max.x, min.y));
        // var corner3 = MakeCorner(glm::vec2(max.x, max.y));
        // var corner4 = MakeCorner(glm::vec2(min.x, max.y));

        var center = MakeCorner(mix(min, max, 0.5));
    }
    */

    if (!button)
        button = GetObjectFromType<ButtonObject>();
}

SpriteObject::SpriteObject()
{
    var initInfo = render::ShaderInitInfo{
        render::SubShader::CreateSubShader("sprite/vert", render::SubShader::Vertex),
        render::SubShader::CreateSubShader("sprite/frag", render::SubShader::Fragment),
    };

    material = new render::Material(render::Shader::CreateShader(initInfo));

    material->state.SetWrite(TMGL_STATE_WRITE_RGB);
    material->state.writeA = true;
    material->state.writeZ = true;
    material->state.depth = render::MaterialState::LessEqual;

    if (!mainTexture)
        mainTexture = fs::ResourceManager::pInstance->loaded_textures["White"];
}

SpriteObject::SpriteObject(string path) :
    SpriteObject()
{
    mainTexture = render::Texture::CreateTexture(path);

    scale = {mainTexture->width, mainTexture->height, 1};
}

void SpriteObject::Update()
{
    var tex = material->GetUniform("s_texColor", true);
    var color = material->GetUniform("u_color", true);
    var spriteData = material->GetUniform("u_spriteData", true);

    var col = mainColor;
    if (copyAlpha)
    {
        var s_parent = GetParent()->Cast<SpriteObject>();
        if (s_parent)
        {
            col.a *= s_parent->mainColor.a;
        }
    }

    color->v4 = col.getData();
    tex->tex = mainTexture;
    spriteData->v4 = glm::vec4(layer, isUI, useAlpha, 0);

    //material->state.write = BGFX_STATE_WRITE_RGB;
    //material->state.depth = render::MaterialState::Always;

    var drawCall = render::DrawCall();

    drawCall.layer = layer + 1;
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
    else
    {


        switch (anchor)
        {
            case TopLeft:
            case CenterLeft:
            case BottomLeft:
                position.x = -position.x;
                position.x += (static_cast<float>(renderer->windowWidth) / 2) - (scale.x / 2);
                break;
            case TopRight:
            case CenterRight:
            case BottomRight:
                position.x -= (static_cast<float>(renderer->windowWidth) / 2) - (scale.x / 2);
                break;
        }

        switch (anchor)
        {
            case TopLeft:
            case TopCenter:
            case TopRight:
                position.y = -position.y;
                position.y += (static_cast<float>(renderer->windowHeight) / 2) - (scale.y / 2);
                break;
            case BottomLeft:
            case BottomCenter:
            case BottomRight:
                position.y -= (static_cast<float>(renderer->windowHeight) / 2) - (scale.y / 2);
                break;
        }
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
        auto _overrides = new render::MaterialOverride[material->overrides.size()];

        std::copy(material->overrides.begin(), material->overrides.end(), _overrides);

        drawCall.overrides = _overrides;
        drawCall.overrideCt = material->overrides.size();
    }
    else
        drawCall.overrides = nullptr;

    pushDrawCall(drawCall);

    for (auto child : children)
    {
        var s = child->Cast<SpriteObject>();
        if (s)
        {
            s->anchor = anchor;
        }
    }

    Object::Update();
}

ButtonObject* SpriteObject::MakeButton()
{
    button = new ButtonObject;
    button->SetParent(this);

    return button;
}

ButtonObject* SpriteObject::GetButton()
{
    return button;
}


void ButtonObject::Start()
{
    var gpos = GetGlobalPosition();
    var gscl = GetGlobalScale();
    var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

    var min = rect.getMin();
    var max = rect.getMax();

    //var corner1 = MakeCorner(glm::vec2(min.x, min.y));
    //var corner2 = MakeCorner(glm::vec2(max.x, min.y));
    //var corner3 = MakeCorner(glm::vec2(max.x, max.y));
    //var corner4 = MakeCorner(glm::vec2(min.x, max.y));

    //var center = MakeCorner(mix(min, max, 0.5));

    sprite = parent->Cast<SpriteObject>();
    origTexture = sprite->mainTexture;

    Object::Start();
}

void ButtonObject::Update()
{
    auto wmp = input::Mouse::GetMousePosition();

    // Retrieve the window dimensions.
    float width = renderer->windowWidth;
    float height = renderer->windowHeight;

    // Adjust the mouse position to NDC (-1 to 1 range).
    glm::vec2 ndcMouse;
    ndcMouse.x = (1.0 - (wmp.x / width)) * 2.0f - 1.0f; // Normalize to [-1, 1]
    ndcMouse.y = (1.0f - (wmp.y / height)) * 2.0f - 1.0f; // Flip Y and normalize to [-1, 1]

    // Apply orthographic projection scaling based on FOV and screen dimensions.
    float rad = glm::radians(mainCamera->FOV);

    if (!application->is2D)
        rad = 1;

    float halfWidth = (width / 2.0f) * rad;
    float halfHeight = (height / 2.0f) * rad;

    // Map NDC coordinates to world space using the orthographic bounds.
    glm::vec2 worldMouse;
    worldMouse.x = ndcMouse.x * halfWidth;
    worldMouse.y = ndcMouse.y * halfHeight;

    var pos = worldMouse;

    //cursor->position = glm::vec3(pos, 0);

    var gpos = GetGlobalPosition();
    var gscl = GetGlobalScale();

    if (sprite)
    {

        switch (sprite->anchor)
        {
            case TopLeft:
            case CenterLeft:
            case BottomLeft:
                gpos.x = -gpos.x;
                gpos.x += (static_cast<float>(renderer->windowWidth) / 2) - (gscl.x / 2);
                break;
            case TopRight:
            case CenterRight:
            case BottomRight:
                gpos.x -= (static_cast<float>(renderer->windowWidth) / 2) - (gscl.x / 2);
                break;
        }

        switch (sprite->anchor)
        {
            case TopLeft:
            case TopCenter:
            case TopRight:
                gpos.y = -gpos.y;
                gpos.y += (static_cast<float>(renderer->windowHeight) / 2) - (gscl.y / 2);
                break;
            case BottomLeft:
            case BottomCenter:
            case BottomRight:
                gpos.y -= (static_cast<float>(renderer->windowHeight) / 2) - (gscl.y / 2);
                break;
        }
    }
    var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

    bool hover = rect.isPointInRect(pos);
    bool click = false;

    // mama a typecheck behind u

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

        var min = rect.getMin();
        var max = rect.getMax();
        var p = pos;
    }

    hoverLast = hover;
    clickLast = click;
}

void ButtonObject::SetParent(Object* o)
{

    if (o->Cast<SpriteObject>())
    {
        sprite = o->Cast<SpriteObject>();
    }

    Object::SetParent(o);
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

TextObject::TextObject()
{
    material->Reload(render::Shader::CreateShader("text/vert", "text/frag"));

    material->state.SetWrite(TMGL_STATE_WRITE_RGB);
    material->state.writeA = true;
    material->state.writeZ = true;
    material->state.depth = render::MaterialState::LessEqual;

    if (!mainTexture)
        mainTexture = fs::ResourceManager::pInstance->loaded_textures["White"];

    mainColor = render::Color::Black;


    //var cursor = MakeCorner(glm::vec2(0));
    //cursor->SetParent(this);
}

void TextObject::Start()
{
    if (spacing == FLT_MAX)
        spacing = font->spacing;

    c = MakeCorner(glm::vec2(position.x, position.y));

    SpriteObject::Start();
}

void TextObject::Update()
{
    var tex = material->GetUniform("s_texColor", true);
    var color = material->GetUniform("u_color", true);

    var col = mainColor;
    if (copyAlpha)
    {
        var s_parent = GetParent()->Cast<SpriteObject>();
        if (s_parent)
        {
            col.a *= s_parent->mainColor.a;
        }
    }

    color->v4 = col.getData();
    tex->tex = mainTexture;

    // material->state.write = BGFX_STATE_WRITE_RGB;
    // material->state.depth = render::MaterialState::Always;

    var gpos = GetGlobalPosition();
    var gscl = GetGlobalScale();

    var og_pos = position;

    if (!isUI)
    {

        position = GetGlobalPosition() - mainCameraObject->position;
        // position.x -= renderer->windowWidth / 2.0f;
        // position.y += renderer->windowHeight / 2.0f;
    }
    else
    {


        switch (anchor)
        {
            case TopLeft:
            case CenterLeft:
            case BottomLeft:
                position.x = -position.x;
                position.x += (static_cast<float>(renderer->windowWidth) / 2) - (gscl.x / 2);
                break;
            case TopRight:
            case CenterRight:
            case BottomRight:
                position.x -= (static_cast<float>(renderer->windowWidth) / 2) - (gscl.x / 2);
                break;
        }

        switch (anchor)
        {
            case TopLeft:
            case TopCenter:
            case TopRight:
                position.y = -position.y;
                position.y += (static_cast<float>(renderer->windowHeight) / 2) - (gscl.y / 2);
                break;
            case BottomLeft:
            case BottomCenter:
            case BottomRight:
                position.y -= (static_cast<float>(renderer->windowHeight) / 2) - (gscl.y / 2);
                break;
        }
    }
    position -= glm::vec3(scale.x / 2, scale.y / 2, 0);

    if (c)
        c->position = position;

    var transform = GetLocalTransform(false);

    if (isUI)
    {
        transform = GetTransform(false);
    }

    position = og_pos;

    float x = 0;
    var textSize = font->CalculateTextSize(text, size, spacing);

    if (HorizontalAlign == Left)
        x -= (textSize / 2);
    else if (HorizontalAlign == Right)
        x += (textSize / 2) + size;
    else if (HorizontalAlign == HCenter)
        x += (size / 2);
    //x = 0;


    float y = 0;

    if (VerticalAlign == Top)
        y -= (textSize / 2);
    else if (VerticalAlign == Bottom)
        y += (textSize / 2) + size;

    float scl = 1;

    //scl = 0.35;

    scl *= (size / 48);

    //scl = size;

    //scl = 1;

    //scl *= -1;

    int textIdx = 0;
    for (char value : text)
    {

        var c = font->characters[value];

        float xPos = x - c.bearing.x * scl;
        float yPos = y - (c.size.y - c.bearing.y) * scl;

        x -= (c.advance >> 6) * scl;

        if (value == ' ' || value == '\0')
        {
            continue;
        }


        var drawCall = render::DrawCall();

        drawCall.layer = layer + 1;
        drawCall.state = material->GetMaterialState();
        drawCall.matrixMode = render::MaterialState::OrthoProj;

        drawCall.transformMatrix = transform;
        drawCall.transformMatrix = translate(drawCall.transformMatrix, glm::vec3(xPos, yPos, 0));
        drawCall.transformMatrix = glm::scale(drawCall.transformMatrix, glm::vec3(scl, scl, 0));

        drawCall.vbh = c.vbh;
        drawCall.ibh = font->ibh;

        drawCall.vertexCount = 4;
        drawCall.indexCount = 6;

        var uni = material->GetUniform("s_fontTex", true);
        uni->tex = c.handle;

        drawCall.program = material->shader;
        if (material->overrides.size() > 0)
        {

            auto _overrides = new render::MaterialOverride[material->overrides.size()];

            std::copy(material->overrides.begin(), material->overrides.end(), _overrides);

            drawCall.overrides = _overrides;
            drawCall.overrideCt = material->overrides.size();
        }
        else
            drawCall.overrides = nullptr;

        pushDrawCall(drawCall);

        textIdx++;
    }

    Object::Update();
}

TextButtonObject::TextButtonObject(Rect r, render::Font* font)
{
    sprite = new SpriteObject();
    button = sprite->MakeButton();
    text = new TextObject();
    text->font = font;

    text->SetParent(sprite);

    sprite->SetParent(this);

    rect = r;
}

void TextButtonObject::Update()
{
    sprite->position.x = rect.x;
    sprite->position.y = rect.y;

    sprite->scale.x = rect.width;
    sprite->scale.y = rect.height;

    //text->position.x = -rect.width / 2;
    //text->position.y = -rect.height / 2;

    Object::Update();
}
