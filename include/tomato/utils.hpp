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



#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef EPSILON
#define EPSILON 0.000001f
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
typedef unsigned int uint;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <numeric>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


using string = std::string;

#endif // TM_UTILS
