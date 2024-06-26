#include "utils.h"

#include "collections.h"
#include "filesystem_tm.h"


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

void RunCommand(const char* cmd)
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
