#if !defined(UTILS_HPP)
#define UTILS_HPP

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


#include "GL/glew.h"
#include <glfw/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <json.hpp>
#include <numeric>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using string = std::string;
namespace fs = std::filesystem;

void RunCommand(const char* cmd);
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

namespace glm {
    void to_json(nlohmann::json& j, const glm::vec2& P);;

    void from_json(const nlohmann::json& j, glm::vec2& P);

    void to_json(nlohmann::json& j, const glm::vec3& P);;

    void from_json(const nlohmann::json& j, glm::vec3& P);
}


#endif // UTILS_HPP

