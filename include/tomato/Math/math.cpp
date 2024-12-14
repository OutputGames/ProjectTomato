#include "math.hpp" 
#include "globals.hpp" 

float *tmt::math::vec4toArray(glm::vec4 v)
{
    float f[4] = {v.x, v.y, v.z, v.w};
    return f;
}

float *tmt::math::mat4ToArray(glm::mat4 m)
{
    float t[4][4];
    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            t[x][y] = m[x][y];
        }
    }

    return reinterpret_cast<float *>(t);
}

float **tmt::math::mat3ToArray(glm::mat3 m)
{
    return nullptr;
}

glm::vec3 tmt::math::slerp(glm::vec3 start, glm::vec3 end, float t)
{
    // Normalize the input vectors
    glm::vec3 startNormalized = normalize(start);
    glm::vec3 endNormalized = normalize(end);

    // Compute the dot product
    float dot = glm::dot(startNormalized, endNormalized);

    // Clamp the dot product to avoid numerical errors
    dot = glm::clamp(dot, -1.0f, 1.0f);

    // Calculate the angle between the vectors
    float theta = acos(dot) * t;

    // Compute the second quaternion using spherical interpolation
    glm::vec3 endPerpendicular = normalize(endNormalized - startNormalized * dot);

    // Perform spherical interpolation and return the result
    return startNormalized * cos(theta) + endPerpendicular * sin(theta);
}

float tmt::math::magnitude(glm::vec3 v)
{
    float m = glm::sqrt(glm::pow(v.x, 2) + glm::pow(v.y, 2) + glm::pow(v.z, 2));

    if (m < 0)
    {
        m = 0;
    }

    return m;
}
