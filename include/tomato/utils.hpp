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

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using ulong = unsigned long long;
using uint = unsigned int;
using ushort = unsigned short;
using byte = unsigned char;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/easing.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/type_aligned.hpp>

inline int randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
}

inline float randomFloat(float min, float max)
{
    return min + (static_cast<float>(rand()) / RAND_MAX) * (max - min);
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

    inline string to_string(glm::quat v)
    {
        return "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," +
            std::to_string(v.w) + ")";
    }
}

#include <stdio.h>
#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
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
#define MAX_BONE_MATRICES 32
#define TO_ARGS(v) v.x, v.y, v.z
#define IN_VECTOR(vec, val) vec.find(val) != vec.end()

#endif // TM_UTILS
