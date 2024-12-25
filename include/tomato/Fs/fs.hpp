#ifndef FS_H
#define FS_H

#include "utils.hpp" 
#include "Audio/audio.hpp"


namespace tmt::render
{
    struct ComputeShader;
    struct Shader;
    struct SubShader;
}

namespace tmt::audio
{
    struct Sound;
}

namespace tmt::fs {

struct StringBinaryReader;
struct BinaryReader;

struct StringBinaryReader : std::stringstream
{
    StringBinaryReader(string data);

    READ_FUNC(u16, UInt16);
    READ_FUNC(u32, UInt32);
    READ_FUNC(u8, Byte);

    template <typename T> T Read()
    {
        T header;
        read(reinterpret_cast<char *>(&header), sizeof(T));

        return header;
    }

    template <typename T> T* ReadArray(int count)
    {
        T* arr = new T[count];

        for (int i = 0; i < count; i++)
        {
            arr[i] = (Read<T>());
        }

        return arr;
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

struct BinaryReader : std::ifstream
{
    enum ByteOrder
    {
        BigEndian,
        LittleEndian
    } byteOrder;

    BinaryReader(std::string path);

    template <typename T> T Read()
    {
        T header;
        read(reinterpret_cast<char *>(&header), sizeof(T));

        return header;
    }

    template <typename T> T* ReadArray(int count)
    {
        T* t = new T[count];

        for (int i = 0; i < count; i++)
        {
            t[i] = Read<T>();
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

    bool CheckSignature(string sig)
    {
        var s = ReadString(sig.size());
        return s == sig;
    }

    glm::vec3 ReadVec3()
    {
        
        var vec = glm::vec3(ReadSingle(), ReadSingle(), ReadSingle());

        std::cout << "reading vec3: " << tellg() << " " << std::to_string(vec) << std::endl;

        return vec;
    }
    glm::vec4 ReadVec4() { return glm::vec4(ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle()); }
    glm::vec2 ReadVec2() { return glm::vec2(ReadSingle(), ReadSingle()); }
    glm::quat ReadQuat()
    {
        var v = ReadVec4();
        return glm::quat(v.w,v.x,v.y,v.z);
    }

    u16* ReadUInt16Array(int size) {
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

    u64 ReadUInt64();
    u32 ReadUInt32();
    u16 ReadUInt16();

    s16 ReadInt16();
    s32 ReadInt32();
    s64 ReadInt64();



    ByteOrder ReadByteOrder();
    unsigned char ReadByte();

    void Skip(int count)
    {
         seekg(count, std::ios::_Seekcur);
    }

    float ReadSingle()
    {
        return Read<float>();
    }



    float* ReadFloatArray(int ct) {
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

    u32 ReadOffset();
    string ReadString(int size);
string ReadString();

    size_t fileSize;

    void SeekBegin(int pos)
    {
        seekg(pos, std::ios::_Seekbeg);
    }

    void Align(int alignment)
    {
        seekg((-tellg() % alignment + alignment) % alignment, _Seekcur);
    }
};
;
#define RESFUNC(name, p1, p2, ret) private: \
        ret _##name##p1; \
    public: \
    inline static ret name##p1 \
    { \
        return pInstance->_##name##p2; \
    } \

    struct ResourceManager
    {
    public:
        static ResourceManager* pInstance;
        ResourceManager();

        RESFUNC(GetSound, (string path, audio::Sound::SoundInitInfo info = {}), (path, info), audio::Sound*);

        std::map<string, tmt::audio::Sound*> loaded_sounds;
        std::map<string, render::SubShader*> loaded_sub_shaders;
        std::map<string, render::Shader*> loaded_shaders;
        std::map<string, render::ComputeShader*> loaded_compute_shaders;

        void ReloadShaders();

        inline void ReloadAssets()
        { ReloadShaders();   
        }

    };

}

#endif