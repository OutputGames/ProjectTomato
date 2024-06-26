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

struct Logger
{

    enum Level {
        INFO = 0,
        DBG,
        WARN,
        LOG_ERROR,
    };

    TMAPI static void Log(std::string msg, Logger::Level logLevel = INFO, std::string source = "Engine");

    inline static std::map<std::pair<int, std::string>, Logger::Level> loggedEntries = std::map<std::pair<int, std::string>, Logger::Level>();

    // Overload << operator for various types
    template <typename T>
    Logger& operator<<(const T& message) {
        if (fileStream.is_open()) {
            fileStream << message;
        }
        else {
            std::cout << message;
            //loggedEntries.insert({ {static_cast<int>(loggedEntries.size()),std::to_string(message)}, INFO });

        }
        return *this;
    }

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

    static Logger logger;

private:
    std::ofstream fileStream;
};


#endif // UTILS_HPP

