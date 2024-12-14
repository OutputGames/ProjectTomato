#include "StringBinaryReader.hpp" 
#include "globals.cpp" 

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

tmt::fs::StringBinaryReader::StringBinaryReader(string data) : std::stringstream(data)
{
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

