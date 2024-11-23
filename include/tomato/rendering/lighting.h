#if !defined(LIGHTING_H)
#define LIGHTING_H

#include "render.h"
#include "util/utils.h"

class tmLight;
class tmCubemap;
struct tmSkybox;

struct TMAPI tmLighting {
    tmLighting();
    ~tmLighting();

    tmSkybox* skybox;

    void SetCubemap(tmCubemap* cubemap);

    struct tmSubLight
    {
        glm::vec3 position, direction, color;
        int lightType;
        float intensity;
        float nearp;
    	float farp;
        float shadowAngle;

        mat4 GetLightSpaceMatrix();

        tmSubLight();

    private:

        friend tmLight;

        mat4 lightSpaceMatrix;
    };

    void Update(tmShader* shader);
    void Draw();

    nlohmann::json Serialize();
    void Deserialize(nlohmann::json j);


    List<tmSubLight*> lights;

    static tmTexture* GetBRDFTexture();
    static void UnloadBRDF();

    tmFramebuffer* depthFBO;

private:
    static void GenerateBRDF();

};

class TMAPI tmCubemap : public tmTexture
{
public:

    tmCubemap(string path, bool flip);
    void unload() override;

    unsigned irradianceId, prefilterId;
    inline static List<tmCubemap*> cb_textures = List<tmCubemap*>();

};

struct TMAPI tmSkybox
{

    tmSkybox(tmCubemap* cubemap = nullptr, tmShader* shader = nullptr);
    ~tmSkybox();

    void use(tmShader* shader);
    void draw(tmBaseCamera* camera);

    tmCubemap* cubemap_;

private:

    friend tmLighting;

    tmShader* shader;

};

class TMAPI tmLight : public Component
{
	CLASS_DECLARATION(tmLight)

public:
	tmLight(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmLight();

    void Update() override;

	enum LightType
	{
		Point,
        Directional,
        Spot
	} lightType = Point;

    glm::vec3 color = glm::vec3(1);
    float intensity = 1;
    float nearp = 0.1f;
    float farp = 1000;
	float shadowAngle = 45;

    void EngineRender() override;

    nlohmann::json Serialize() override;
    void Deserialize(nlohmann::json j) override;

private:

    tmLighting::tmSubLight* subLight;

};

#endif // LIGHTING_H
