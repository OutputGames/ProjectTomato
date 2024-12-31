#include "fs.hpp"

#include <ranges>

#include "globals.hpp"

using namespace tmt::fs;


BinaryWriter::BinaryWriter(std::string path) :
    std::ofstream(path, std::ios::binary)
{

}

BinaryReader::BinaryReader(std::string path) :
    std::ifstream(path, std::ios::binary)
{
    fileSize = tellg();

    if constexpr (std::endian::native == std::endian::little)
    {
        std::cout << "Little-endian\n";
    }
    else if constexpr (std::endian::native == std::endian::big)
    {
        std::cout << "Big-endian\n";
    }
    else
    {
        std::cout << "Mixed-endian\n"; // Unlikely
    }
}

u64 BinaryReader::ReadUInt64()
{
    return Read<u64>();
}

u32 BinaryReader::ReadUInt32()
{
    return Read<u32>();
}

u16 BinaryReader::ReadUInt16()
{
    return Read<u16>();
}

s16 BinaryReader::ReadInt16()
{
    return Read<s16>();
}

s32 BinaryReader::ReadInt32()
{
    return Read<s32>();
}

s64 BinaryReader::ReadInt64()
{
    return Read<s64>();
}

BinaryReader::ByteOrder BinaryReader::ReadByteOrder()
{
    byteOrder = BigEndian;
    var bom = Read<ByteOrder>();
    byteOrder = bom;
    return bom;
}

unsigned char BinaryReader::ReadByte()
{
    return Read<unsigned char>();
}

u32 BinaryReader::ReadOffset()
{
    var offset = static_cast<u32>(ReadUInt64());
    return offset == 0 ? 0 : offset;
}

string BinaryReader::ReadString(int size)
{
    string s = "";

    for (int i = 0; i < size; ++i)
    {
        s += Read<char>();
    }

    return s;
}

string BinaryReader::ReadString()
{
    var size = ReadInt32();
    return ReadString(size);
}

ResourceManager* ResourceManager::pInstance;

ResourceManager::ResourceManager() { pInstance = this; }

void ResourceManager::ReloadShaders()
{
    for (auto shader : loaded_shaders | std::views::values)
    {
        shader->Reload();
    }
}
