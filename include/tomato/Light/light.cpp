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
        pos[i] = (glm::vec4(0));
        col[i] = (glm::vec4(0));
    }

    setUniform(position, pos);
    setUniform(color, col);

    glm::vec4 lightData(lights.size(), 1, 1, 1);

    setUniform(data, lightData);


}

tmt::light::LightUniforms::LightUniforms()
{
    position = createUniform("iu_lightPosition", tmgl::UniformType::Vec4, maxLights);
    direction = createUniform("iu_lightDirection", tmgl::UniformType::Vec4, maxLights);
    color = createUniform("iu_lightColor", tmgl::UniformType::Vec4, maxLights);
    power = createUniform("iu_lightPower", tmgl::UniformType::Vec4, maxLights);
    data = createUniform("iu_lightData", tmgl::UniformType::Vec4);
}

tmt::light::LightObject::LightObject()
{
    light = new Light;
}

void tmt::light::LightObject::Update()
{
    light->position = GetGlobalPosition();
    light->direction = GetForward();
    light->color = color;

    render::pushLight(light);

    Object::Update();
}

tmt::light::SkyboxObject::SkyboxObject()
{
    var shader = render::Shader::CreateShader("test/cube_vert", "test/cube_frag");
    skyboxMaterial = new render::Material(shader);

    skyboxMesh = GetPrimitive(prim::Cube);
}

void tmt::light::SkyboxObject::Update()
{
    if (cubemap)
    {

        skyboxMaterial->GetUniform("map")->tex = cubemap->realTexture;

        skyboxMesh->draw(glm::mat4(1.0), skyboxMaterial, glm::vec3{0}, layer);
    }


    Object::Update();
}
