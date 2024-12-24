#if !defined(TM_UTILS)
#define TM_UTILS

#ifdef TOMATO_DLLBUILD
#define TMAPI __declspec(dllexport)
#else
#define TMAPI __declspec(dllimport)
#endif

#define var auto
#define flt (float)
#define arrsize(a) sizeof(a) / sizeof(a[0])
#define randval(min, max) (rand()%(abs(max - min) + 1) + min)

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>


#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef EPSILON
#define EPSILON 1e-5f
#endif

#ifndef DEG2RAD
#define DEG2RAD (PI/180.0f)
#endif

#ifndef RAD2DEG
#define RAD2DEG (180.0f/PI)
#endif
#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef unsigned long long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char byte;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/easing.hpp>
#include <glm/gtx/type_aligned.hpp>
#include <glm/matrix.hpp>

inline int randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
}

inline float randomFloat(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

inline void CombineVectors(std::vector<glm::vec3>& v1, std::vector<glm::vec3>& v2)
{
    v1.insert(v1.end(), v2.begin(), v2.end());
}

using string = std::string;

namespace std
{
    inline string to_string(glm::vec3 v)
    {
        return "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")";
    }
}

#include <stdio.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3.h>
#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <random>

#include <assimp/Importer.hpp>

#include <bx/math.h>

#include <GLFW/glfw3native.h>

#include "vertex.h"
#include "bgfx/src/debug_renderdoc.h"
#include "tomato/common/debugdraw/debugdraw.h"

#include "bullet3/src/btBulletDynamicsCommon.h"

#include "vertex.h"

#define READ_FUNC(type,name) type Read##name() { return Read<##type>(); };
#define MAX_BONE_MATRICES 250
#define TO_ARGS(v) v.x, v.y, v.z
#define IN_VECTOR(vec, val) vec.find(val) != vec.end()

#endif // TM_UTILS
