#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gShading;

in vec2 TexCoords;
in vec3 Normal;
in vec4 WorldPosition;
in vec4 ScreenPosition;

uniform sampler2D texture1;

void main()
{
    gAlbedoSpec = texture(texture1, TexCoords);
    gPosition = vec3(WorldPosition);
    gNormal = normalize(Normal);
    gShading = vec4(1,0,0,0);
    //gAlbedoSpec = WorldPosition;

    //FragColor = vec4(Normal, 1.0);

}