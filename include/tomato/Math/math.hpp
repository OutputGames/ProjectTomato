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

    inline glm::vec3 convertVec3(float* arr)
    { return glm::vec3{arr[0], arr[1], arr[2]};
    }

    inline glm::vec4 convertVec4(float* arr, int arrSize)
    {

        var v = glm::vec4();

        for (int i = 0; i < arrSize; ++i)
        {
            v[i] = arr[i];
        }

        return v;
    }
    inline glm::ivec4 convertIVec4(int* arr, int arrSize)
    {

        var v = glm::ivec4(-1);

        for (int i = 0; i < arrSize; ++i)
        {
            v[i] = arr[i];
        }

        return v;
    }

        inline glm::vec2 convertVec2(float* arr) { return glm::vec2{arr[0], arr[1] }; }

inline glm::vec3 convertVec3(aiVector3D v)
{
    return glm::vec3{v.x, v.y, v.z};
};

inline glm::quat convertQuat(aiQuaternion v)
{
    return glm::quat{v.w,v.x, v.y, v.z};
};


glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float t);

glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t);
float lerp(float start, float end, float t);

float magnitude(glm::vec3 v);
;

}

#endif