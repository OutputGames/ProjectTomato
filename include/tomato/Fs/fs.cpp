#include "fs.hpp" 
#include "globals.hpp" 

using namespace tmt::fs;

tmt::fs::StringBinaryReader::StringBinaryReader(string data) : std::stringstream(data)
{
}

string tmt::fs::StringBinaryReader::fLoadString(u32 offset)
{
    var p = tellg();
    seekg(offset, _Seekbeg);

    u16 size = ReadUInt16();

    string s = ReadString();
    seekg(p, _Seekbeg);

    return s;
}

string tmt::fs::StringBinaryReader::ReadString()
{
    // short size = ReadInt16();
    return ReadUtf8();
}

string tmt::fs::StringBinaryReader::ReadUtf8()
{
    long start = tellg();
    int size = 0;

    while (ReadByte() - 1 > 0 && size < INT_MAX)
    {
        size++;
    }

    seekg(start, _Seekbeg);
    var text = ReadString(size);
    seekg(1, _Seekcur);
    return text;
}

string tmt::fs::StringBinaryReader::ReadString(int size)
{
    string s = "";

    for (int i = 0; i < size; ++i)
    {
        s += Read<char>();
    }

    return s;
}


tmt::fs::BinaryReader::BinaryReader(std::string path) : std::ifstream(path, std::ios::binary)
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

u64 tmt::fs::BinaryReader::ReadUInt64()
{
    return Read<u64>();
}

u32 tmt::fs::BinaryReader::ReadUInt32()
{
    return Read<u32>();
}

u16 tmt::fs::BinaryReader::ReadUInt16()
{
    return Read<u16>();
}

s16 tmt::fs::BinaryReader::ReadInt16()
{
    return Read<s16>();
}

s32 tmt::fs::BinaryReader::ReadInt32()
{
    return Read<s32>();
}

s64 tmt::fs::BinaryReader::ReadInt64()
{
    return Read<s64>();
}

tmt::fs::BinaryReader::ByteOrder tmt::fs::BinaryReader::ReadByteOrder()
{
    byteOrder = BigEndian;
    var bom = Read<ByteOrder>();
    byteOrder = bom;
    return bom;
}

unsigned char tmt::fs::BinaryReader::ReadByte()
{
    return Read<unsigned char>();
}

u32 tmt::fs::BinaryReader::ReadOffset()
{
    var offset = static_cast<u32>(ReadUInt64());
    return offset == 0 ? 0 : offset;
}

string tmt::fs::BinaryReader::ReadString(int size)
{
    string s = "";

    for (int i = 0; i < size; ++i)
    {
        s += Read<char>();
    }

    return s;
}

string tmt::fs::BinaryReader::ReadString()
{
    var size = ReadInt32();
    return ReadString(size);
}

std::map<string, tmt::audio::Sound*> loaded_sounds;
ResourceManager* ResourceManager::pInstance;

tmt::fs::ResourceManager::ResourceManager()
{ pInstance = new ResourceManager; }

tmt::audio::Sound* tmt::fs::ResourceManager::GetSound(string path)
{
    if (loaded_sounds.find(path) != loaded_sounds.end())
{
        return loaded_sounds[path];
}
else
{
    var sound = new tmt::audio::Sound(path);
    loaded_sounds[path] = sound;
    return sound;
}
}
