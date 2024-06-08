#include "utils.h"

#include "filesystem_tm.h"

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

char* tmfs::loadFileBytes(string path, int& ds)
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