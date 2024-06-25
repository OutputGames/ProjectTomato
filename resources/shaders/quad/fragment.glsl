#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;
uniform sampler2D shading;
uniform vec3 clearColor;

void main()
{
    vec4 shad = texture(shading, TexCoord);
    vec4 color = texture(albedo,TexCoord);
    vec3 normal = texture(normal,TexCoord).rgb;
    vec3 position = texture(position,TexCoord).rgb;

    //FragColor = vec4(clearColor,1.0);

    if (shad.a == 1) 
        FragColor = vec4(clearColor,1.0);
    else {

        vec3 lightColor = vec3(0.7804, 0.6863, 0.4157);
        vec3 lightPos = vec3(1,1,1);

        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;

        vec3 lightDir = normalize(lightPos - position);  

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 result = (ambient + diffuse) * vec3(color) ;
        FragColor = vec4(result, 1.0);
    }

    //FragColor = vec4(color);
}