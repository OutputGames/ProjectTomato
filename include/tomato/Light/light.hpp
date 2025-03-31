#if !defined(LIGHT_HPP)
#define LIGHT_HPP

#include "utils.hpp"
#include "Obj/obj.hpp"
#include "Render/render.hpp"

namespace tmt::light
{
    struct Light
    {
        glm::vec3 position;
        glm::vec3 direction;
        float power;

        render::Color color;
    };

    struct LightUniforms
    {
        tmgl::UniformHandle position, direction, color, power, data;
        const u16 maxLights = 10;

        void Apply(std::vector<Light*> lights);

        LightUniforms();
    };

    struct LightObject : obj::Object
    {

        LightObject();
        void Update() override;

        Light* light;
        render::Color color = render::Color::White;

    };

    struct SkyboxObject : obj::Object
    {
        SkyboxObject();

        void Update() override;

        render::CubemapTexture* cubemap = nullptr;

    private:
        render::Material* skyboxMaterial = nullptr;
        render::Mesh* skyboxMesh = nullptr;
    };

}

#endif // LIGHT_HPP
