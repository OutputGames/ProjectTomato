#include "math.hpp" 
#include "globals.hpp" 

float *tmt::math::vec4toArray(glm::vec4 v)
{
    float f[4] = {v.x, v.y, v.z, v.w};
    return f;
}

float* tmt::math::mat4ToArray(glm::mat4 m)
{
    float* t = new float[16];
    int i = 0;
    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            t[i] = m[x][y];
            i++;
        }
    }

    return t;
}

float **tmt::math::mat3ToArray(glm::mat3 m)
{
    return nullptr;
}

float* tmt::math::mat4ArrayToArray(std::vector<glm::mat4> v)
{
    float* arr = new float[v.size() * 16];
    int j = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                arr[j] = v[i][x][y];
                j++;
            }
        }
    }

    return arr;
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

glm::vec3 tmt::math::lerp(glm::vec3 start, glm::vec3 end, float t)
{ return glm::mix(start, end, t); }

float tmt::math::lerp(float start, float end, float t)
{
    return glm::lerp(start, end, t);
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
