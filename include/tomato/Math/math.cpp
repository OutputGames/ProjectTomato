#include "math.hpp"
#include "globals.hpp"

float* tmt::math::vec4toArray(glm::vec4 v)
{
    float f[4] = {v.x, v.y, v.z, v.w};
    return f;
}

float* tmt::math::mat4ToArray(glm::mat4 m)
{
    auto t = new float[16];
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

float** tmt::math::mat3ToArray(glm::mat3 m)
{
    return nullptr;
}

float** tmt::math::mat4ArrayToArray(std::vector<glm::mat4> v)
{
    std::vector<float*> m4(v.size());
    int j = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        m4[j] = value_ptr(v[i]);
    }

    return m4.data();
}

glm::quat tmt::math::LookRotation(glm::vec3 forward, glm::vec3 up)
{
    glm::vec3 f = normalize(forward); // Ensure forward is normalized
    glm::vec3 r = normalize(cross(up, f)); // Right vector
    glm::vec3 u = cross(f, r); // Corrected up vector

    glm::mat3 rotationMatrix(r, u, f); // Right, Up, Forward form the basis vectors
    return quat_cast(rotationMatrix); // Convert to quaternion
}

glm::quat tmt::math::FromToRotation(glm::vec3 from, glm::vec3 to)
{
    glm::vec3 f = normalize(from);
    glm::vec3 t = normalize(to);

    float dotProduct = glm::dot(f, t);
    if (dotProduct >= 1.0f)
    {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // No rotation needed
    }
    if (dotProduct <= -1.0f)
    {
        // Handle 180-degree rotation
        glm::vec3 axis = normalize(cross(glm::vec3(1, 0, 0), f));
        if (glm::length(axis) < 0.001f)
        {
            axis = normalize(cross(glm::vec3(0, 1, 0), f));
        }
        return angleAxis(glm::pi<float>(), axis);
    }

    glm::vec3 crossProduct = cross(f, t);
    float s = glm::sqrt((1 + dotProduct) * 2);
    float invS = 1 / s;

    return glm::quat(s * 0.5f, crossProduct.x * invS, crossProduct.y * invS, crossProduct.z * invS);
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
{
    return mix(start, end, t);
}

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
