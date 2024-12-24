#include "light.hpp"

#include "Math/math.hpp"

void tmt::light::LightUniforms::Apply(std::vector<Light*> lights)
{
    std::vector<glm::vec4> pos(maxLights);
    std::vector<glm::vec4> col(maxLights);
    for (int i = 0; i < lights.size(); ++i)
    {
        var light = lights[i];
        pos[i] = (glm::vec4(light->position, 0));
        col[i] = (light->color.getData());
    }

    for (int i = lights.size(); i < maxLights; ++i)
    {
        pos[i] = (glm::vec4( 0));
        col[i] = (glm::vec4(0));
    }

    bgfx::setUniform(position, pos);
    bgfx::setUniform(color, col);

    glm::vec4 lightData(lights.size(), 1, 1, 1);

    bgfx::setUniform(data, lightData);
}

tmt::light::LightUniforms::LightUniforms()
{
    position = bgfx::createUniform("iu_lightPosition", bgfx::UniformType::Vec4, maxLights);
    direction = bgfx::createUniform("iu_lightDirection", bgfx::UniformType::Vec4, maxLights);
    color = bgfx::createUniform("iu_lightColor", bgfx::UniformType::Vec4, maxLights);
    power = bgfx::createUniform("iu_lightPower", bgfx::UniformType::Vec4, maxLights);
    data = bgfx::createUniform("iu_lightData", bgfx::UniformType::Vec4);
}

tmt::light::LightObject::LightObject()
{ light = new Light; }

void tmt::light::LightObject::Update()
{
    light->position = GetGlobalPosition();
    light->direction = GetForward();
    light->color = color;

    tmt::render::pushLight(light);

    Object::Update();
}
