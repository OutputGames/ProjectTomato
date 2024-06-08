#if !defined(UTILS_HPP)
#define UTILS_HPP

#if defined(_WIN32)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(BUILD_LIBTYPE_SHARED)
        #define TMAPI __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define TMAPI __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#else
    #if defined(BUILD_LIBTYPE_SHARED)
        #define TMAPI __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef TMAPI
    #define TMAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif


#define var auto
#define flt (float)

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


#include "GL/glew.h"
#include <glfw/glfw3.h>

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

#endif // UTILS_HPP
