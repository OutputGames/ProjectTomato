#include "lighting.h"

#include "engine.hpp"

#include "misc/debug.h"

#include "stb/stb_image.h"

CLASS_DEFINITION(Component, tmLight)

inline static tmTexture* brdfTexture = nullptr;

tmLight::tmLight()
{
    subLight = new tmLighting::tmSubLight;
}

void tmLight::Update()
{
    subLight->position = transform()->position;

    glm::vec3 rotation = transform()->rotation;

    glm::vec3 direction;
    direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    direction.y = sin(glm::radians(rotation.x));
    direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

    if (lightType == Point)
    {
        direction = -(subLight->position - vec3(0.0));
    }

    subLight->direction = direction;

    subLight->lightType = lightType;
    subLight->color = color;
    subLight->intensity = intensity;
    subLight->lightSpaceMatrix = mat4(0.0);
    subLight->nearp = nearp;
    subLight->farp = farp;
    subLight->shadowAngle = shadowAngle;
}

void tmLight::EngineRender()
{
    glm::vector_color("Color", color);

    const char* items[] = { "Point", "Directional", "Spot" };

    if (ImGui::BeginCombo("lighttype_combo", items[lightType]))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (lightType == n);
            if (ImGui::Selectable(items[n], is_selected))
                lightType = (LightType)n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::DragFloat("Intensity", &intensity, 0.1f);

    ImGui::DragFloat("Shadow Near", &nearp, 0.1f);
    ImGui::DragFloat("Shadow Far", &farp, 0.1f);
    ImGui::DragFloat("Shadow Angle", &shadowAngle);

    switch (lightType)
    {
    case Point:
        tmglDebugRenderer::sphere(transform()->GetGlobalPosition(), glmtodd(color), intensity/10);
        break;
    case Directional:
        tmglDebugRenderer::cone(transform()->GetGlobalPosition(), -transform()->GetForward(), 1.0f, 2.0f, glmtodd(color));
        break;
    case Spot:
        tmglDebugRenderer::cone(transform()->GetGlobalPosition(), -transform()->GetForward(), 0.0f, 2.0f, glmtodd(color));
        break;
    }

}

nlohmann::json tmLight::Serialize()
{
    json j = {};

	glm::to_json(j["color"], color);
    j["intensity"] = intensity;
    j["type"] = lightType;

    return j;
}

void tmLight::Deserialize(nlohmann::json j)
{

    intensity = j["intensity"];

    glm::from_json(j["color"], color);

    lightType = j["type"];

}

mat4 tmLighting::tmSubLight::GetLightSpaceMatrix()
{
    if (lightSpaceMatrix == mat4(0.0))
    {

        glm::vec3 rotation;

        rotation.x = glm::degrees(asin(direction.y)); // Pitch
        rotation.y = glm::degrees(atan2(direction.z, direction.x)); // Yaw
        rotation.z = 0.0f; // Roll (assuming no roll)

        float near_plane = nearp, far_plane = farp;
        glm::mat4 lightProjection;

        lightProjection = glm::perspective(glm::radians(shadowAngle), (GLfloat)1024 / (GLfloat)1024, near_plane, far_plane);

        vec3 up = glm::quat(rotation) * vec3(0,1,0);

        vec3 dir(0.0);

        if (lightType == tmLight::Directional)
        {
            dir = direction;
            lightProjection = glm::ortho(-1.f,1.f,-1.f,1.f, near_plane, far_plane);
        }

        glm::mat4 lightView = glm::lookAt(position,
            dir,
            vec3(0,1,0));

        lightSpaceMatrix = lightProjection * lightView;
    }

    return lightSpaceMatrix;
}

const char* vertcode0 = R"(

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;  
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}

)";

const char* fragcode0 = R"(

#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 GetEnvironmentLight(vec3 p)
{
			
	vec4 GroundColour = vec4(0.6);
	vec4 SkyColourHorizon=vec4(1);
	vec4 SkyColourZenith=vec4(0.65,0.85,1,1);
	float SunFocus=01;
	float SunIntensity=0;
	vec3 SunPos=vec3(10,10,10);
						
	float skyGradientT = pow(smoothstep(0, 0.4, p.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0, p.y);
	vec3 skyGradient = mix(vec3(SkyColourHorizon), vec3(SkyColourZenith), skyGradientT);
	float sun = pow(max(0, dot(p, SunPos)), SunFocus) * SunIntensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(vec3(GroundColour), skyGradient, groundToSkyT) + sun * float(groundToSkyT>=1);
	return composite;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    //vec3 color = GetEnvironmentLight(WorldPos);

    FragColor = vec4(color, 1.0);
}

)";

const char* vertcode1 = R"(

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos;
}  

)";

const char* fragcode1 = R"(


#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 3) out vec4 shad;
layout (location = 4) out vec4 lpos;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    vec3 envColor = texture(skybox, TexCoords).rgb;

    //envColor = envColor / (envColor + vec3(1.0));
    //envColor = pow(envColor, vec3(1.0/2.2)); 
  
    FragColor = vec4(envColor, 1.0);
	shad = vec4(0.0);
	lpos = vec4(0.0);

}

)";

const char* irradiancecode = R"(

#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}

)";

const char* quadvtx = R"(

#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;


out vec3 vpos;
out vec2 TexCoords;

void main(){

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;
    vpos = vertexPosition_modelspace;
    TexCoords = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;;
}

)";

const char* prefiltercode = R"(
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(WorldPos);
    
    // make the simplifying assumption that V equals R equals the normal 
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}
)";

const char* brdfcode = R"(

#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // note that we use a different k for IBL
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
// ----------------------------------------------------------------------------
void main() 
{
    vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    FragColor = vec4(integratedBRDF, 0, 1.0);
    //FragColor = vec4(1);
}

)";

tmLighting::tmLighting()
{
    skybox = new tmSkybox;

    GenerateBRDF();

    int FBO_SIZE = 1024;

    depthFBO = new tmFramebuffer(tmFramebuffer::DepthOnly, vec2(FBO_SIZE));


}

tmLighting::~tmLighting()
{
    delete skybox;
    brdfTexture->unload();

    for (auto light : lights)
        delete light;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO2 = 0;
unsigned int cubeVBO2 = 0;
void renderCube2()
{
    // initialize (if necessary)
    if (cubeVAO2 == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO2);
        glGenBuffers(1, &cubeVBO2);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO2);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO2);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void tmLighting::SetCubemap(tmCubemap* cubemap)
{
    skybox->cubemap_ = cubemap;
}

tmLighting::tmSubLight::tmSubLight()
{
    tmeGetCore()->lighting->lights.Add(this);
}
void tmLighting::Update(tmShader* shader)
{
    int i = 0;
    for (auto light : lights.GetVector())
    {

        shader->setVec3("lights["+ std::to_string(i) +"].position", light->position);
        shader->setVec3("lights["+ std::to_string(i) +"].direction", light->direction);
        shader->setVec3("lights["+ std::to_string(i) +"].color", light->color);
        shader->setInt("lights["+ std::to_string(i) +"].type", light->lightType);
        shader->setInt("lights["+ std::to_string(i) +"].enabled", true);
        shader->setFloat("lights["+ std::to_string(i) +"].intensity", light->intensity);
        shader->setFloat("lights["+ std::to_string(i) +"].nearPlane", light->nearp);
        shader->setFloat("lights["+ std::to_string(i) +"].farPlane", light->farp);
        shader->setFloat("lights["+ std::to_string(i) +"].shadowAngle", light->shadowAngle);
        shader->setMat4("lightMatrices["+ std::to_string(i) +"]", light->GetLightSpaceMatrix());

    	i++;
    }


}
void tmLighting::Draw()
{

    static tmShader* simpleDepthShader;

    if (!simpleDepthShader)
    {

        string dv = R"(

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

const int MAX_BONES = 252;
const int MAX_BONE_INFLUENCE_NOREN = 4;
uniform mat4 boneTransforms_NOREN[MAX_BONES];

void main()
{

    vec4 updatedPosition = vec4(0.0f);
    vec3 updatedNormal = vec3(0.0f);
    
    for(int i = 0 ; i < MAX_BONE_INFLUENCE_NOREN; i++)
    {
        // Current bone-weight pair is non-existing
        if(boneIds[i] == -1) 
            continue;

        // Ignore all bones over count MAX_BONES
        if(boneIds[i] >= MAX_BONES) 
        {
            updatedPosition = vec4(aPos,1.0);
            break;
        }

        mat4 boneTransform = mat4(1.0);
        
        mat4 ts = mat4(1.0);

        boneTransform = boneTransforms_NOREN[boneIds[i]];
        //col += boneTransform[boneIds[i]];

        // Set pos
        vec4 localPosition = boneTransform * vec4(aPos,1.0);
        updatedPosition += localPosition * weights[i];
        // Set normal
        vec3 localNormal = mat3(boneTransform) * aNormal;
        updatedNormal += localNormal * weights[i];
    }

    if (updatedPosition == vec4(0)) {
        //updatedPosition = vec4(aPos,1.0);
    }

    if (boneIds == ivec4(-1)) {
        updatedPosition = vec4(aPos,1.0);
    }

    if (updatedNormal == vec3(0)) {
        updatedNormal = aNormal;
    }

    gl_Position = lightSpaceMatrix * model * updatedPosition;
}  

    	)";
        string df = R"(

#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}  

)";

        simpleDepthShader = new tmShader(dv, df, false);
    }

    depthFBO->use();

    var renderMgr = tmeGetCore()->renderMgr;

    simpleDepthShader->use();

    var lightSpaceMatrix = lights[0]->GetLightSpaceMatrix();

    simpleDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    for (auto draw_call : renderMgr->drawCalls)
    {
        simpleDepthShader->setMat4("model", draw_call->modelMatrix);
        simpleDepthShader->setMat4Array("boneTransforms_NOREN", draw_call->boneMats);
        glBindVertexArray(draw_call->buffer->VAO);
        if (draw_call->buffer->EBO != -1)
        {
            glDrawElements(GL_TRIANGLES, draw_call->buffer->elementCount, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(draw_call->buffer->mode, 0, draw_call->buffer->vertexCount);
        }
        glBindVertexArray(0);
    }
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

nlohmann::json tmLighting::Serialize()
{

    json j = {};

    {

        json s = {};
        s["path"] = skybox->cubemap_->path;
        s["flip"] = skybox->cubemap_->flip;

        s["shader"]["vertex"] = skybox->shader->vertData;
        s["shader"]["fragment"] = skybox->shader->fragData;
        s["shader"]["isPath"] = skybox->shader->isPath;

        j["skybox"] = s;

    }


    return j;


}

void tmLighting::Deserialize(nlohmann::json j)
{

    string skyboxPath = j["skybox"]["path"];
    bool skyboxFlip = j["skybox"]["flip"];

    string skyboxShaderVert = j["skybox"]["shader"]["vertex"];
    string skyboxShaderFrag = j["skybox"]["shader"]["fragment"];
    bool skyboxShaderIsPath = j["skybox"]["shader"]["isPath"];


    skybox = new tmSkybox(new tmCubemap(skyboxPath, skyboxFlip), new tmShader(skyboxShaderVert, skyboxShaderFrag, skyboxShaderIsPath));

}

tmTexture* tmLighting::GetBRDFTexture()
{
    if (!brdfTexture)
        GenerateBRDF();

    return brdfTexture;
}

void tmLighting::UnloadBRDF()
{
    brdfTexture = nullptr;
    GenerateBRDF();
}

void tmLighting::GenerateBRDF()
{
    if (brdfTexture)
        brdfTexture->unload();

    brdfTexture = new tmTexture(new tmShader(quadvtx, brdfcode, false), 512, 512, GL_RG);
}

tmCubemap::tmCubemap(string path, bool flip)
{
	type = CUBE;
    engineId = cb_textures.GetCount();
    this->path = path;
    this->flip = flip;

    tmShader* equirectangularToCubemapShader = new tmShader(vertcode0,fragcode0,false);
    tmShader* irradianceShader = new tmShader(vertcode0, irradiancecode, false);
    tmShader* prefilterShader = new tmShader(vertcode0, prefiltercode, false);

    // pbr: setup framebuffer
// ----------------------
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // pbr: load the HDR environment map
    // ---------------------------------
    stbi_set_flip_vertically_on_load(flip);
    int width, height, nrComponents;
    unsigned int hdrTexture;
    if (HasExtension(path, "hdr")) {
        float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            Logger::Log("Failed to load texture at " + path, Logger::LOG_ERROR, "TEXTURE");
        }
    } else
    {
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {

            GLenum format;

            switch (nrComponents)
            {
            case 1:
                format = GL_R16F;
                break;
            case 2:
                format = GL_RG16F;
                break;
            case 3:
                format = GL_RGB16F;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                break;
            }

            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            Logger::Log("Failed to load texture at " + path, Logger::LOG_ERROR, "TEXTURE");
        }
        stbi_image_free(data);
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangularToCubemapShader->use();
    equirectangularToCubemapShader->setInt("equirectangularMap", 0);
    equirectangularToCubemapShader->setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube2();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
// --------------------------------------------------------------------------------
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
// -----------------------------------------------------------------------------
    irradianceShader->use();
    irradianceShader->setInt("environmentMap", 0);
    irradianceShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube2();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilterShader->use();
    prefilterShader->setInt("environmentMap", 0);
    prefilterShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube2();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    id = envCubemap;
    irradianceId = irradianceMap;
    prefilterId = prefilterMap;
    cb_textures.Add(this);

}

void tmCubemap::unload()
{
	tmTexture::unload();

    glDeleteTextures(1, &irradianceId);
        glDeleteTextures(1, &prefilterId);
}

tmSkybox::tmSkybox(tmCubemap* cubemap, tmShader* shader)
{
    cubemap_ = cubemap;
    if (shader)
    {
        this->shader = shader;
    } else
    {
        this->shader = new tmShader(vertcode1, fragcode1,false);

        this->shader->setInt("skybox", 0);
    }
}

tmSkybox::~tmSkybox()
{
    cubemap_->unload();
    shader->unload();
}

void tmSkybox::use(tmShader* shader)
{
    
}

void tmSkybox::draw(tmBaseCamera* camera)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    shader->use();

    glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix()));
    camera->UpdateShader(shader);

    shader->setMat4("view", view);

    if (cubemap_) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_->id);
    }

    renderCube2();

    glDepthMask(GL_TRUE);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
}
