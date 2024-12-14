#ifndef MATH_H
#define MATH_H

#include "utils.hpp" 





namespace tmt::math {


float *vec4toArray(glm::vec4 v);

float *mat4ToArray(glm::mat4 m);

float **mat3ToArray(glm::mat3 m);

inline bx::Vec3 convertVec3(glm::vec3 v)
{
    return bx::Vec3{v.x, v.y, v.z};
};

inline glm::vec3 convertVec3(aiVector3D v)
{
    return glm::vec3{v.x, v.y, v.z};
};

glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float t);

glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t);

float magnitude(glm::vec3 v);
;

}

#endif