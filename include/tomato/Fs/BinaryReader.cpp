#include "BinaryReader.hpp" 
#include "globals.cpp" 

tmt::fs::BinaryReader::BinaryReader(std::streambuf *data) : std::istream(data)
{
    fileSize = tellg();
}

tmt::fs::BinaryReader::ByteOrder tmt::fs::BinaryReader::ReadByteOrder()
{
    byteOrder = BigEndian;
    var bom = Read<ByteOrder>();
    byteOrder = bom;
    return bom;
}

u64 tmt::fs::BinaryReader::ReadUInt64()
{
    return Read<u64>();
}

u8 tmt::fs::BinaryReader::ReadByte()
{
    return Read<u8>();
}

u16 tmt::fs::BinaryReader::ReadUInt16()
{
    return Read<u16>();
}

u32 tmt::fs::BinaryReader::ReadUInt32()
{
    return Read<u32>();
}

u32 tmt::fs::BinaryReader::ReadOffset()
{
    var offset = static_cast<u32>(ReadUInt64());
    return offset == 0 ? 0 : offset;
}

s16 tmt::fs::BinaryReader::ReadInt16()
{
    return Read<s16>();
}

s64 tmt::fs::BinaryReader::ReadInt64()
{
    return Read<s64>();
}

s32 tmt::fs::BinaryReader::ReadInt32()
{
    return Read<s32>();
}

