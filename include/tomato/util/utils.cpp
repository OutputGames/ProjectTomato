
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shlobj.h>
#else
#include <cstdlib>
#endif

#include "utils.h"

#include "collections.h"
#include "engine.hpp"
#include "filesystem_tm.h"
#include "input.h"
#include "rendering/tmgl.h"



Logger Logger::logger = Logger("");

string tmfs::loadFileString(string path)
{
    const std::ifstream input_stream(path, std::ios_base::binary);

    if (input_stream.fail()) {
        throw std::runtime_error("Failed to open file located at " + path);
    }

    std::stringstream buffer;
    buffer << input_stream.rdbuf();

    return buffer.str();
}

char* tmfs::loadFileBytes(string path, uint32_t& ds)
{
    std::ifstream stream(path, std::ios::binary | std::ios::ate);

    if (!stream)
    {
        // Failed to open the file
        return nullptr;
    }

    std::streampos end = stream.tellg();
    stream.seekg(0, std::ios::beg);
    uint32_t size = end - stream.tellg();

    if (size == 0)
    {
        // File is empty
        return nullptr;
    }

    char* buffer = new char[size];
    stream.read((char*)buffer, size);
    stream.close();

    ds = size;
    return buffer;
}

void tmfs::writeFileString(string path, string data)
{
    std::ofstream out(path);
    out << data;
    out.close();
}

void tmfs::copyFile(string from, string to)
{
    std::ifstream  src(from, std::ios::binary);
    std::ofstream  dst(to, std::ios::binary);

    dst << src.rdbuf();
}

bool tmfs::fileExists(string path)
{
	return std::filesystem::exists(path);
}

void tmfs::copyDirectory(string from, string to)
{
    std::filesystem::create_directories(to);

    // Iterate through the source directory
    for (const auto& entry : fs::recursive_directory_iterator(from)) {
        const auto& path = entry.path();
        auto relative_path = fs::relative(path, from);
        fs::path dest_path = to / relative_path;

        if (fs::is_directory(path)) {
            fs::create_directories(dest_path);
        }
        else if (fs::is_regular_file(path)) {
            fs::copy_file(path, dest_path, fs::copy_options::overwrite_existing);
        }
    }
}

string tmfs::getHomeDirectory()
{
    const char* homeDir = nullptr;

#if defined(_WIN32) || defined(_WIN64)
    char homePath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, LPWSTR(homePath)))) {
        homeDir = homePath;
    }
#else
    homeDir = getenv("HOME");
#endif

    return homeDir;
}

extern "C" {

    TMAPI void RunCommand(const char* cmd)
    {
        Logger::logger << "Running command: \n\t" << cmd << std::endl;

        system(cmd);
    }

    std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    bool HasExtension(const std::string& filename, const std::string& extension)
    {
        if (filename.length() >= extension.length()) {
            return (0 == filename.compare(filename.length() - extension.length(), extension.length(), extension));
        }
        else {
            return false;
        }
    }

    bool StringContains(const std::string& str, const std::string& substr)
    {
        return str.find(substr) != std::string::npos;
    }

}

void glm::to_json(nlohmann::json& j, const int& p)
{
    j = p;
}

void glm::from_json(const nlohmann::json& j, int& p)
{
    p = j;
}

void glm::to_json(nlohmann::json& j, const float& p)
{
    j = p;
}

void glm::from_json(const nlohmann::json& j, float& p)
{
    p = j;
}

void glm::to_json(nlohmann::json& j, const bool& p)
{
    j = p;
}

void glm::from_json(const nlohmann::json& j, bool& p)
{
    p = j;
}

void glm::to_json(nlohmann::json& j, const vec4& P)
{
    j = { { "x", P.x }, { "y", P.y }, {"z", P.z}, {"w", P.w} };
}

void glm::from_json(const nlohmann::json& j, vec4& P)
{
    P.x = j.at("x").get<double>();
    P.y = j.at("y").get<double>();
    P.z = j.at("z").get<double>();
    P.w = j.at("w").get<double>();
}

void glm::to_json(nlohmann::json& j, const mat2& p)
{
    to_json(j["x"], p[0]);
    to_json(j["y"], p[1]);
}

void glm::from_json(const nlohmann::json& j, mat2& p)
{
    from_json(j["x"], p[0]);
    from_json(j["y"], p[1]);
}

void glm::to_json(nlohmann::json& j, const mat3& p)
{
    to_json(j["x"], p[0]);
    to_json(j["y"], p[1]);
    to_json(j["z"], p[2]);
}

void glm::from_json(const nlohmann::json& j, mat3& p)
{
    from_json(j["x"], p[0]);
    from_json(j["y"], p[1]);
    from_json(j["z"], p[2]);
}

void glm::to_json(nlohmann::json& j, const mat4& p)
{
    to_json(j["x"], p[0]);
    to_json(j["y"], p[1]);
    to_json(j["z"], p[2]);
    to_json(j["w"], p[3]);
}

void glm::from_json(const nlohmann::json& j, mat4& p)
{
    from_json(j["x"], p[0]);
    from_json(j["y"], p[1]);
    from_json(j["z"], p[2]);
    from_json(j["w"], p[3]);
}

void glm::vector_color(string name, vec2& p)
{
    float vals[3];
    vals[0] = p.x;
    vals[1] = p.y;
    vals[2] = 1;

    ImGui::ColorEdit3(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];

}

void glm::vector_drag(string name, vec2& p)
{
    float vals[2];
    vals[0] = p.x;
    vals[1] = p.y;

    ImGui::DragFloat2(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];
}

void glm::vector_color(string name, vec3& p)
{
    float vals[3];
    vals[0] = p.x;
    vals[1] = p.y;
    vals[2] = p.z;

    ImGui::ColorEdit3(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
}

void glm::vector_drag(string name, vec3& p)
{
    float vals[3];
    vals[0] = p.x;
    vals[1] = p.y;
    vals[2] = p.z;

    ImGui::DragFloat3(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
}

void glm::vector_color(string name, vec4& p)
{
    float vals[4];
    vals[0] = p.x;
    vals[1] = p.y;
    vals[2] = p.z;
    vals[3] = p.w;

    ImGui::ColorEdit4(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
    p.w = vals[3];
}

void glm::vector_drag(string name, vec4& p)
{
    float vals[4];
    vals[0] = p.x;
    vals[1] = p.y;
    vals[2] = p.z;
    vals[3] = p.w;

    ImGui::DragFloat4(name.c_str(), vals);

    p.x = vals[0];
    p.y = vals[1];
    p.z = vals[2];
    p.w = vals[3];
}

glm::mat4 glm::mulmatSIMD(const glm::mat4& mat1, const glm::mat4& mat2)
{
    /*
    glm::mat4 result;

    const float* m1 = glm::value_ptr(mat1);
    const float* m2 = glm::value_ptr(mat2);
    float* res = glm::value_ptr(result);

    __m128 row1 = _mm_loadu_ps(&m2[0]);
    __m128 row2 = _mm_loadu_ps(&m2[4]);
    __m128 row3 = _mm_loadu_ps(&m2[8]);
    __m128 row4 = _mm_loadu_ps(&m2[12]);

    for (int i = 0; i < 4; ++i) {
        __m128 brod1 = _mm_set1_ps(m1[i * 4 + 0]);
        __m128 brod2 = _mm_set1_ps(m1[i * 4 + 1]);
        __m128 brod3 = _mm_set1_ps(m1[i * 4 + 2]);
        __m128 brod4 = _mm_set1_ps(m1[i * 4 + 3]);
        __m128 row = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(brod1, row1),
                _mm_mul_ps(brod2, row2)),
            _mm_add_ps(
                _mm_mul_ps(brod3, row3),
                _mm_mul_ps(brod4, row4)));
        _mm_storeu_ps(&res[i * 4], row);
    }

    return result;
    */

    return mat1 * mat2;
}

void Logger::Log(std::string msg, Logger::Level logLevel, std::string source)
{
	Logger::logger << msg,logLevel,source;
}

void glm::to_json(nlohmann::json& j, const glm::vec2& P)
{
	j = { { "x", P.x }, { "y", P.y } };
}

void glm::from_json(const nlohmann::json& j, glm::vec2& P)
{
	P.x = j.at("x").get<double>();
	P.y = j.at("y").get<double>();
}

void glm::to_json(nlohmann::json& j, const glm::vec3& P)
{
	j = { { "x", P.x }, { "y", P.y }, {"z", P.z} };
}

void glm::from_json(const nlohmann::json& j, glm::vec3& P)
{
	P.x = j.at("x").get<double>();
	P.y = j.at("y").get<double>();
	P.z = j.at("z").get<double>();
}


void tmInput::pollEvents()
{
    keys_pressed.Clear();
    keys_released.Clear();
    keys_repeated.Clear();
}

void tmInput::update()
{
    if (!_attached)
    {
        glfwSetKeyCallback(tmGetCore()->window, key_callback);
        glfwSetCursorPosCallback(tmGetCore()->window, cursor_callback);
        _attached = true;
    }
}

void tmInput::key_callback(GLFWwindow* window, int tmkey, int scancode, int action, int mods)
{

	switch (action)
	{
	case GLFW_PRESS:
        keys_pressed.Add(tmkey);
        break;
	case GLFW_REPEAT:
        keys_repeated.Add(tmkey);
        break;
	case GLFW_RELEASE:
        keys_released.Add(tmkey);
        break;
	}

    ImGui_ImplGlfw_KeyCallback(window, tmkey, scancode, action, mods);

	//std::cout << "Key interacted: scancode " << scancode << std::endl;

}

void tmInput::cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (tmEngine::tmTime::time <= 2) {
        mouseDelta = { xpos,ypos };
    }
    else {
        mouseDelta = mousePosition - glm::vec2{ -xpos, ypos };
    }
    mousePosition = { xpos,ypos };
    glm::vec2 windowPos = tmGetCore()->getWindowPos();
    mousePosition += windowPos;

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

void tmInput::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scrollOffset = { xoffset,yoffset };
}

glm::vec2 tmInput::getMousePosition()
{
	return mousePosition;
}

glm::vec2 tmInput::getMouseDelta()
{
    return mouseDelta;
}

glm::vec2 tmInput::getScrollOffset()
{
    return scrollOffset;
}

int tmInput::getKey(int key)
{
	return glfwGetKey(tmGetCore()->window, key);
}

int tmInput::getMouseButton(int button)
{
    return glfwGetMouseButton(tmGetCore()->window, button);
}
