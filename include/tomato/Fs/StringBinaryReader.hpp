#ifndef STRINGBINARYREADER_H
#define STRINGBINARYREADER_H

#include "utils.hpp" 



namespace tmt::fs {

struct StringBinaryReader : std::stringstream
{
    StringBinaryReader(string data);

    READ_FUNC(u16, UInt16);
    READ_FUNC(u8, Byte);

    template <typename T> T Read()
    {
        T header;
        read(reinterpret_cast<char *>(&header), sizeof(T));

        return header;
    }

    template <typename T> std::vector<T> ReadArray(int count)
    {
        std::vector<T> t;

        for (int i = 0; i < count; i++)
        {
            t.push_back(Read<T>());
        }

        return t;
    }

    template <typename T> std::vector<T> ReadArray(ulong offset, int count)
    {
        long p = tellg();

        SeekBegin(offset);
        var list = ReadArray<T>(count);

        SeekBegin(p);
        return list;
    }

    void SeekBegin(int pos)
    {
        seekg(pos, std::ios::_Seekbeg);
    }

    void Align(int alignment)
    {
        seekg((-tellg() % alignment + alignment) % alignment, _Seekcur);
    }

    string fLoadString(u32 offset);
    string ReadString();
    string ReadUtf8();
    string ReadString(int size);
};

}

#endif