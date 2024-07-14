#version 330 core
out vec4 FragColor;


struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    int type;
    bool enabled;
    float intensity;
    float nearPlane;
    float farPlane;
    float shadowAngle;
};

#define LIGHT_NR 100
const float PI = 3.14159265359;

in vec2 TexCoord;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D position;
uniform sampler2D shading;
uniform sampler2D shadowPosition;
uniform samplerCube skybox;
uniform samplerCube irradiance;
uniform samplerCube prefilter;
uniform sampler2D   brdfLUT;  
uniform sampler2D shadowMap;

uniform vec3 clearColor;
uniform vec3 cameraPosition;
uniform Light lights[LIGHT_NR];

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 GetAmbient(vec4 color) {
    float ambientStrength = 0.03;
    vec3 ambient = texture(irradiance,texture(normal,TexCoord).rgb).rgb;

    vec3 I = normalize( texture(position,TexCoord).rgb - cameraPosition);
    vec3 R = reflect(I, normalize( texture(normal,TexCoord).rgb));
    //ambient += ambientStrength/2 * texture(skybox, R).rgb;
 
    return ambient;
}

vec3 CalcLight(Light light,vec3 lightDir, vec3 N, vec3 V, vec3 pos, vec3 color, vec3 shad) {

    float roughness = shad.g;
    float metallic = shad.b;
    vec3 albedo = color;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);

    vec3 L = (lightDir);
    vec3 H = normalize(V + L);

    float dist = length(lightDir);

    float attenuation = 1.0 / (dist * dist);

    if (light.type == 1)
        attenuation = 1;
    
    vec3 radiance = light.color * attenuation * light.intensity;

    float NDF = DistributionGGX(N,H,roughness);
    float G = GeometrySmith(N,V,L,roughness);
    vec3 F = fresnelSchlick(clamp(dot(H,V),0.0,1.0),F0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N,L),0.0);
    vec3 result = (kD * albedo / PI + specular) * radiance * NdotL;

    //result = vec3(NdotL);

    return result;
}

vec3 CalcPoint(Light light, vec3 normal, vec3 pos, vec3 V, vec3 color, vec3 shad) {

    vec3 lightDir = normalize(light.position - pos);
    float dist = length(light.position - pos);
    
    return CalcLight(light,lightDir,normal, V, pos, color, shad);
}

vec3 CalcDir(Light light, vec3 normal, vec3 viewDir, vec3 color, vec3 shad) {
    vec3 lightDir = normalize(-light.direction);
    float dist = length(-light.direction);

    return CalcLight(light,lightDir, normal,viewDir, texture(position,TexCoord).rgb, color, shad);

}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow

    Light light = lights[0];

    vec3 normal = texture(normal,TexCoord).rgb;
    vec3 lightDir = normalize(-lights[0].direction);

    if (light.type == 0) {
        lightDir = normalize(light.position - texture(position,TexCoord).xyz);
    }

    lightDir = normalize(light.position - texture(position,TexCoord).xyz);

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    bias = 0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    if(projCoords.z > light.farPlane)
        shadow = 0.0;

    return shadow;
}  

void main()
{
    vec4 shad = texture(shading, TexCoord);
    vec4 col = texture(albedo,TexCoord);

    col = pow(col, vec4(vec3(2.2),1));

    float normalA = texture(normal,TexCoord).a;
    vec3 normal = texture(normal,TexCoord).rgb;
    vec3 position = texture(position,TexCoord).rgb;

    //FragColor = vec4(clearColor,1.0);


    if (normalA > 0.5) {
        FragColor = vec4(vec3( pow(vec3(col),vec3(1.0/2.2)) ),1);
        //discard;
    }
    else {

        vec3 viewDir = normalize(cameraPosition - position);

        vec3 result = vec3(0);

        vec4 dataColor = vec4(vec3(col), shad.g);

        for (int i = 0; i < LIGHT_NR; i++) {

            Light light = lights[i];

            if (light.enabled == false)
                continue;

            //light.type = 1;

            switch (light.type) {

                case 0:
                    result += CalcPoint(light, normal, position, viewDir, dataColor.rgb, shad.rgb);
                    break;
                case 1:
                    result += CalcDir(light,normal,viewDir,dataColor.rgb, shad.rgb);
                    break;
                case 2:
                    break;

            }
            
        }

        vec3 F0 = vec3(0.04);
        F0 = mix(F0,col.rgb,shad.b);

        // ambient lighting (we now use IBL as the ambient term)
        vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, shad.g);
        
        vec3 kS = F;
        vec3 kD = (1.0) - kS;
        kD *= 1.0 - shad.b;

        vec3 irradianceValue = texture(irradiance,normal).rgb;
        vec3 diffuse = irradianceValue * col.rgb;

        const float MAX_REFL_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilter, reflect(-viewDir,normal), shad.g * MAX_REFL_LOD).rgb;
        vec2 envBRDF = texture(brdfLUT, vec2(max(dot(normal,viewDir),0.0),shad.g)).rg;
        vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

        float shadow = ShadowCalculation(texture(shadowPosition, TexCoord)) + 0.05;

        vec3 ambient = (kD * diffuse + specular);
        vec3 color = ambient + ((1.0 -shadow) * result);

        color = color / (color + vec3(1.0));
        color = pow(color,vec3(1.0/2.2));

        FragColor = vec4(color, 1.0);

    
        //FragColor = vec4(vec3(shadow),1.0);
    }
    

    //FragColor = vec4(vec3(texture(albedo,TexCoord)),1.0);
}