#ifndef FS_H
#define FS_H

#include <codecvt>

#include "utils.hpp"

#include <sstream>

namespace tmt::render
{
    struct Font;
    struct SceneDescription;
    struct Mesh;
    struct Texture;
    struct ComputeShader;
    struct Shader;
    struct SubShader;
}

namespace tmt::audio
{
    struct Sound;
}

namespace tmt::fs
{
    struct BinaryReader;
    struct BinaryWriter;

    struct BinaryWriter : std::ofstream
    {
        BinaryWriter(std::string path) :
            std::ofstream(path, std::ios::binary)
        {
        }

        template <typename T>
        void Write(T value)
        {
            write(reinterpret_cast<const char*>(&value), sizeof(value));
        }

        template <typename T>
        void Write(T value, int size)
        {
            write(reinterpret_cast<const char*>(&value), size);
        }

        void WriteInt32(u32 i)
        {
            Write(i);
        }

        void WriteVec3(glm::vec3 v)
        {
            Write(v.x);
            Write(v.y);
            Write(v.z);
        }

        void WriteQuat(glm::quat v)
        {
            Write(v.x);
            Write(v.y);
            Write(v.z);
            Write(v.w);
        }

        void WriteVec4(glm::vec4 v)
        {
            Write(v.x);
            Write(v.y);
            Write(v.z);
            Write(v.w);
        }


        void WriteInt16(u16 i)
        {
            Write(i);
        }

        void WriteByte(u8 i)
        {
            Write(i);
        }

        void WriteString(string s)
        {
            WriteInt32(s.size());
            WriteSignature(s);
        }

        void WriteSignature(string s)
        {
            write(s.c_str(), s.size());
        }

        void WriteSingle(float f)
        {
            Write(f);
        }

        void Close()
        {
            close();
        }

    };

    struct BinaryDataReader
    {
        enum ByteOrder
        {
            BigEndian,
            LittleEndian
        } byteOrder = LittleEndian;

        // BinaryReader(std::string path);

        template <class T>
        void endswap(T* objp)
        {
            auto memp = reinterpret_cast<unsigned char*>(objp);
            std::reverse(memp, memp + sizeof(T));
        }

        template <typename T>
        T Read()
        {
            T value;
            std::memcpy(&value, data + position_, sizeof(T));
            position_ += sizeof(T);

            if (byteOrder == BigEndian)
            {
                endswap(&value);
            }

            return value;
        }

        template <typename T>
        T* ReadArray(int count)
        {
            T* t = new T[count];

            for (int i = 0; i < count; i++)
            {
                t[i] = Read<T>();
            }

            return t;
        }

        template <typename T>
        std::vector<T> ReadArray(ulong offset, int count)
        {
            long p = tellg();

            SeekBegin(offset);
            var list = ReadArray<T>(count);

            SeekBegin(p);
            return list;
        }

        bool CheckSignature(string sig)
        {
            var s = ReadString(sig.size());
            return s == sig;
        }

        glm::vec3 ReadVec3()
        {
            float x = ReadSingle();
            float y = ReadSingle();
            float z = ReadSingle();
            var vec = glm::vec3(x, y, z);
            return vec;
        }

        glm::vec4 ReadVec4()
        {
            float x = ReadSingle();
            float y = ReadSingle();
            float z = ReadSingle();
            float w = ReadSingle();
            var vec = glm::vec4(x, y, z, w);
            return vec;
        }

        glm::vec2 ReadVec2() { return glm::vec2(ReadSingle(), ReadSingle()); }

        glm::quat ReadQuat()
        {
            var v = ReadVec4();

            return glm::quat(v.w, v.x, v.y, v.z);
        }

        u16* ReadUInt16Array(int size)
        {
            var arr = new u16[size];

            for (int i = 0; i < size; ++i)
            {
                arr[size] = ReadUInt16();
            }

            return arr;
        }

        u8* ReadByteArray(int size)
        {
            var arr = new u8[size];

            for (int i = 0; i < size; ++i)
            {
                arr[size] = ReadByte();
            }

            return arr;
        }


        void Skip(int count) { seekg(count, std::ios::_Seekcur); }

        float ReadSingle() { return Read<float>(); }


        float* ReadFloatArray(int ct)
        {
            var arr = new float[ct];

            for (int i = 0; i < ct; ++i)
            {
                arr[i] = ReadSingle();
            }

            return arr;
        }

        float ReadHalfFloat() { return ReadSingle(); }

        s8 ReadSByte() { return Read<s8>(); }

        size_t fileSize;

        void SeekBegin(int pos) { seekg(pos, std::ios::_Seekbeg); }

        void SeekCurrent(int pos) { seekg(pos, std::ios::_Seekcur); }

        void Align(int alignment) { seekg((-tellg() % alignment + alignment) % alignment, std::ios::_Seekcur); }

        BinaryDataReader(char* data)
        {
            this->data = data;

        }

        u64 ReadUInt64() { return Read<u64>(); }

        u32 ReadUInt32() { return Read<u32>(); }

        u16 ReadUInt16() { return Read<u16>(); }

        s16 ReadInt16() { return Read<s16>(); }

        s32 ReadInt32() { return Read<s32>(); }

        s64 ReadInt64() { return Read<s64>(); }

        ByteOrder ReadByteOrder()
        {
            byteOrder = BigEndian;
            var bom = Read<ByteOrder>();
            byteOrder = bom;
            return bom;
        }

        unsigned char ReadByte() { return Read<unsigned char>(); }

        u32 ReadOffset()
        {
            var offset = static_cast<u32>(ReadUInt64());
            return offset == 0 ? 0 : offset;
        }

        string ReadString(int size)
        {
            string s = "";

            for (int i = 0; i < size; ++i)
            {
                s += Read<char>();
            }

            return s;
        }

        string ReadString()
        {
            var size = ReadInt32();
            return ReadString(size);
        }

    private:
        size_t tellg()
        {
            return position_;
        }

        void seekg(size_t position, std::ios_base::seekdir dir)
        {
            switch (dir)
            {
                case std::ios::_Seekbeg:
                    position_ = position;
                    break;
                case std::ios::_Seekcur:
                    position_ += position;
                    break;
                default:
                    break;
            }
        }

        const char* data;
        size_t size_;
        size_t position_ = 0;
    };

    struct BinaryReader : std::ifstream
    {
        enum ByteOrder
        {
            BigEndian,
            LittleEndian
        } byteOrder = LittleEndian;

        //BinaryReader(std::string path);

        template <class T>
        void endswap(T* objp)
        {
            auto memp = reinterpret_cast<unsigned char*>(objp);
            std::reverse(memp, memp + sizeof(T));
        }

        template <typename T>
        T Read()
        {
            T header;
            read(reinterpret_cast<char*>(&header), sizeof(T));
            if (byteOrder == BigEndian)
            {
                endswap(&header);
            }

            return header;
        }

        template <typename T>
        T* ReadArray(int count)
        {
            T* t = new T[count];

            for (int i = 0; i < count; i++)
            {
                t[i] = Read<T>();
            }

            return t;
        }

        template <typename T>
        std::vector<T> ReadArray(ulong offset, int count)
        {
            long p = tellg();

            SeekBegin(offset);
            var list = ReadArray<T>(count);

            SeekBegin(p);
            return list;
        }

        bool CheckSignature(string sig)
        {
            var s = ReadString(sig.size());
            return s == sig;
        }

        glm::vec3 ReadVec3()
        {
            float x = ReadSingle();
            float y = ReadSingle();
            float z = ReadSingle();
            var vec = glm::vec3(x, y, z);
            return vec;
        }

        glm::vec4 ReadVec4()
        {
            float x = ReadSingle();
            float y = ReadSingle();
            float z = ReadSingle();
            float w = ReadSingle();
            var vec = glm::vec4(x, y, z, w);
            return vec;
        }

        glm::vec2 ReadVec2() { return glm::vec2(ReadSingle(), ReadSingle()); }

        glm::quat ReadQuat()
        {
            var v = ReadVec4();

            return glm::quat(v.w, v.x, v.y, v.z);
        }

        u16* ReadUInt16Array(int size)
        {
            var arr = new u16[size];

            for (int i = 0; i < size; ++i)
            {
                arr[size] = ReadUInt16();
            }

            return arr;
        }

        unsigned char* ReadByteArray(int size)
        {
            var arr = new unsigned char[size];

            for (int i = 0; i < size; ++i)
            {
                arr[size] = ReadByte();
            }

            return arr;
        }


        void Skip(int count)
        {
            seekg(count, std::ios::_Seekcur);
        }

        float ReadSingle()
        {
            return Read<float>();
        }


        float* ReadFloatArray(int ct)
        {
            var arr = new float[ct];

            for (int i = 0; i < ct; ++i)
            {
                arr[i] = ReadSingle();
            }

            return arr;
        }

        float ReadHalfFloat()
        {
            return ReadSingle();
        }

        s8 ReadSByte()
        {
            return Read<s8>();
        }


        size_t fileSize;

        void SeekBegin(int pos)
        {
            seekg(pos, std::ios::_Seekbeg);
        }

        void SeekCurrent(int pos) { seekg(pos, std::ios::_Seekcur); }

        void Align(int alignment)
        {
            seekg((-tellg() % alignment + alignment) % alignment, _Seekcur);
        }

        BinaryReader(std::string path) :
            std::ifstream(path, std::ios::binary) { fileSize = tellg(); }

        u64 ReadUInt64() { return Read<u64>(); }

        u32 ReadUInt32() { return Read<u32>(); }

        u8 ReadU8() { return Read<u8>(); }

        u16 ReadUInt16() { return Read<u16>(); }

        s16 ReadInt16() { return Read<s16>(); }

        s32 ReadInt32() { return Read<s32>(); }

        s64 ReadInt64() { return Read<s64>(); }

        ByteOrder ReadByteOrder()
        {
            byteOrder = BigEndian;
            var bom = Read<ByteOrder>();
            byteOrder = bom;
            return bom;
        }

        unsigned char ReadByte() { return Read<unsigned char>(); }

        u32 ReadOffset()
        {
            var offset = static_cast<u32>(ReadUInt64());
            return offset == 0 ? 0 : offset;
        }

        string ReadString(int size)
        {
            string s = "";

            for (int i = 0; i < size; ++i)
            {
                s += Read<char>();
            }

            return s;
        }

        string ReadString(int size, int offset)
        {

            SeekBegin(offset);

            return ReadString(size);
        }

        std::string ReadStringUTF16(int size)
        {
            var s = std::u16string();

            for (int i = 0; i < size; ++i)
            {
                char16_t c = ReadUInt16();
                if (c == u'\0')
                    break;
                s.push_back(c);
            }

            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
            return converter.to_bytes(s);
        }

        std::string ReadStringUTF16(int size, int offset)
        {
            SeekBegin(offset);

            return ReadStringUTF16(size);
        }

        string ReadString()
        {
            var size = ReadInt32();
            return ReadString(size);
        }
    };

#define RESFUNC(name, p1, p2, ret) private: \
        ret _##name##p1; \
    public: \
    inline static ret name##p1 \
    { \
        return pInstance->_##name##p2; \
    }

    struct ResourceManager
    {
        static ResourceManager* pInstance;
        ResourceManager();


        std::map<string, audio::Sound*> loaded_sounds;
        std::map<string, render::SubShader*> loaded_sub_shaders;
        std::map<string, render::Shader*> loaded_shaders;
        std::map<string, render::ComputeShader*> loaded_compute_shaders;
        std::map<string, render::Texture*> loaded_textures;
        std::map<string, render::Font*> loaded_fonts;
        std::map<string, render::Mesh*> loaded_meshes;
        std::map<string, render::SceneDescription*> loaded_scene_descs;

        void ReloadShaders();

        void ReloadAssets()
        {
            ReloadShaders();
        }

    };

}

#endif
