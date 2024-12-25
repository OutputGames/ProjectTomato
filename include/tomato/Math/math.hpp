#ifndef MATH_H
#define MATH_H

#include "utils.hpp" 





namespace tmt::math {


float *vec4toArray(glm::vec4 v);

float* mat4ToArray(glm::mat4 m);

float **mat3ToArray(glm::mat3 m);

    float** mat4ArrayToArray(std::vector<glm::mat4> m);

inline bx::Vec3 convertVec3(glm::vec3 v)
{
    return bx::Vec3{v.x, v.y, v.z};
};

    inline float* convertVec3A(glm::vec3 v) {
        var f = new float[3];
        f[0] = v[0];
        f[1] = v[1];
        f[2] = v[2];

        return f;
    }

    inline float* convertVec4A(glm::vec4 v)
    {
        var f = new float[4];
        f[0] = v[0];
        f[1] = v[1];
        f[2] = v[2];
        f[3] = v[3];
 
        return f;
    }

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
inline glm::mat4 convertMat4(const aiMatrix4x4 from)
{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
}

    inline glm::mat4 convertMat4(float from[4][4])
{
    glm::mat4 m;

    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            m[x][y] = from[x][y];
        }
    }

    return m;
}

glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float t);

glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t);
float lerp(float start, float end, float t);

float magnitude(glm::vec3 v);
;

}

#endif