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

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

typedef nlohmann::json json;

extern "C" {
    TMAPI void RunCommand(const char* cmd);
    TMAPI std::string  ReplaceAll(std::string str, const std::string& from, const std::string& to);
    TMAPI bool HasExtension(const std::string& filename, const std::string& extension);
bool StringContains(const std::string& str, const std::string& substr);
}

#define tfjson(type) \
        void to_json(nlohmann::json& j, const type& p); \
		void from_json(const nlohmann::json& j, type& p); \

#define vigui(type) \
		void vector_color(string name, type& p); \
		void vector_drag(string name, type& p); \

namespace glm {
		tfjson(int)
        tfjson(float)
        tfjson(bool)

        tfjson(vec2)
        tfjson(vec3)
        tfjson(vec4)

        tfjson(mat2);
        tfjson(mat3);
        tfjson(mat4);

        vigui(vec2)
        vigui(vec3)
        vigui(vec4)


	glm::mat4 mulmatSIMD(const glm::mat4& mat1, const glm::mat4& mat2);
}

struct Logger
{

    enum Level {
        INFO = 0,
        DBG,
        WARN,
        LOG_ERROR,
    };

    TMAPI static void Log(std::string msg, Logger::Level logLevel = INFO, std::string source = "Engine");

	std::map<std::pair<int, std::string>, Logger::Level> loggedEntries = std::map<std::pair<int, std::string>, Logger::Level>();


    Logger& operator<<(const string& message);

    // Overload << operator for std::endl
	Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (fileStream.is_open()) {
            manip(fileStream);
        }
        else {
            manip(std::cout);
        }
        return *this;
    }

    // Constructor that takes an optional file name for logging to a file
    Logger(const std::string& filename = "") {
        if (!filename.empty()) {
            fileStream.open(filename, std::ios::out | std::ios::app);
        }
    }

    Logger() = default;

    // Destructor
    ~Logger() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    static TMAPI Logger logger;

private:
    std::ofstream fileStream;
};


inline Logger& Logger::operator<<(const string& message)
{
	if (fileStream.is_open()) {
		fileStream << message;
	}
	else {
		std::cout << message;
		loggedEntries.insert({ {static_cast<int>(loggedEntries.size()),(message)}, INFO });

	}
	return *this;
}


#endif // UTILS_HPP

