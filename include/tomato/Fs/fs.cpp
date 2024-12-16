#include "fs.hpp" 
#include "globals.hpp" 

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

u8 tmt::fs::BinaryReader::ReadByte()
{
    return Read<u8>();
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
