#if !defined(TESTPROJ_HPP)
#define TESTPROJ_HPP

#include "tomato/tomato.hpp"

struct Player : tmt::obj::Object
{
    Player();

    float Speed = 10.0f;
    float SwimSpeed = 5.0f;

    void Update() override;

private:

    tmt::obj::CameraObject* cam;
    float camAmt = 90;
    float camY = 0;

    float shootTimer = 0;

    tmt::obj::Object* Mesh;

    tmt::obj::Object* orientation;
    tmt::obj::MeshObject* testForward, *testUp, *testRight;

    tmt::physics::PhysicsBody* body;

    tmt::particle::ParticleEmitter* emitter;

    bool res0 = false;

};

struct PaintableMap : tmt::obj::Object
{
    PaintableMap(int paintMapSize=1024);

    void Update() override;

    void RenderPaintMap(glm::mat4 spaceMatrix, glm::vec3 paintDir);

    tmt::render::RenderTexture* paintMap, *readBackMap;
    tmt::render::Shader* paintShader;
    tmt::obj::MeshObject* Mesh = nullptr;
    tmt::render::Texture* stain;

    tmt::physics::PhysicsBody* body;

    size_t PaintMapSize=1024;
    static const size_t ReadbackDataSize = 8;

    static glm::mat4 CalculatePaintMatrix(glm::vec3 pos, glm::vec3 dir, glm::vec3 right);

    glm::vec4 GetPosition(glm::vec3 p, int faceId);


    tmt::render::Texture* colorBuffer, *colorBufferRB;
    tmt::render::ComputeShader* readBackShader;
    float readbackData[ReadbackDataSize * ReadbackDataSize * 4];

};

struct Paintball : tmt::obj::Object
{

    Paintball();
    void Update();

private:

    tmt::physics::PhysicsBody* body;
    bool exploded = false;
    glm::vec3 direction;
    int epc = 0;

};


struct BfresLoader
{
    static tmt::obj::ObjectLoader::SceneInfo Load(string path);

    class MemoryStreamBuf : public std::streambuf {
    public:
        MemoryStreamBuf(const std::vector<char>& buffer) {
            // Set the input buffer and initialize pointers
            setg(const_cast<char*>(buffer.data()),  // beginning of buffer
                const_cast<char*>(buffer.data()),  // current position
                const_cast<char*>(buffer.data() + buffer.size()));  // end of buffer
        }
        MemoryStreamBuf(const std::vector<unsigned char>& buffer) {
            // Set the input buffer and initialize pointers
            setg((char*)const_cast<unsigned char*>(buffer.data()),  // beginning of buffer
                (char*)const_cast<unsigned char*>(buffer.data()),  // current position
                (char*)const_cast<unsigned char*>(buffer.data() + buffer.size()));  // end of buffer
        }
        MemoryStreamBuf(const unsigned char* buffer, size_t bufferSize) {
            // Set the input buffer and initialize pointers
            setg((char*)const_cast<unsigned char*>(buffer),  // beginning of buffer
                (char*)const_cast<unsigned char*>(buffer),  // current position
                (char*)const_cast<unsigned char*>(buffer + bufferSize));  // end of buffer
        }
    };

    enum class BfresAttribFormat : ushort
    {
        // 8 bits (8 x 1)
        Format_8_UNorm = 0x00000102, //
        Format_8_UInt = 0x00000302, //
        Format_8_SNorm = 0x00000202, //
        Format_8_SInt = 0x00000402, //
        Format_8_UIntToSingle = 0x00000802,
        Format_8_SIntToSingle = 0x00000A02,
        // 8 bits (4 x 2)
        Format_4_4_UNorm = 0x00000001,
        // 16 bits (16 x 1)
        Format_16_UNorm = 0x0000010A,
        Format_16_UInt = 0x0000020A,
        Format_16_SNorm = 0x0000030A,
        Format_16_SInt = 0x0000040A,
        Format_16_Single = 0x0000050A,
        Format_16_UIntToSingle = 0x00000803,
        Format_16_SIntToSingle = 0x00000A03,
        // 16 bits (8 x 2)
        Format_8_8_UNorm = 0x00000109, //
        Format_8_8_UInt = 0x00000309, //
        Format_8_8_SNorm = 0x00000209, //
        Format_8_8_SInt = 0x00000409, //
        Format_8_8_UIntToSingle = 0x00000804,
        Format_8_8_SIntToSingle = 0x00000A04,
        // 32 bits (16 x 2)
        Format_16_16_UNorm = 0x00000112, //
        Format_16_16_SNorm = 0x00000212, //
        Format_16_16_UInt = 0x00000312,
        Format_16_16_SInt = 0x00000412,
        Format_16_16_Single = 0x00000512, //
        Format_16_16_UIntToSingle = 0x00000807,
        Format_16_16_SIntToSingle = 0x00000A07,
        // 32 bits (10/11 x 3)
        Format_10_11_11_Single = 0x00000809,
        // 32 bits (8 x 4)
        Format_8_8_8_8_UNorm = 0x0000010B, //
        Format_8_8_8_8_SNorm = 0x0000020B, //
        Format_8_8_8_8_UInt = 0x0000030B, //
        Format_8_8_8_8_SInt = 0x0000040B, //
        Format_8_8_8_8_UIntToSingle = 0x0000080B,
        Format_8_8_8_8_SIntToSingle = 0x00000A0B,
        // 32 bits (10 x 3 + 2)
        Format_10_10_10_2_UNorm = 0x0000000B,
        Format_10_10_10_2_UInt = 0x0000090B,
        Format_10_10_10_2_SNorm = 0x0000020E, // High 2 bits are UNorm //
        Format_10_10_10_2_SInt = 0x0000099B,
        // 64 bits (16 x 4)
        Format_16_16_16_16_UNorm = 0x00000115, //
        Format_16_16_16_16_SNorm = 0x00000215, //
        Format_16_16_16_16_UInt = 0x00000315, //
        Format_16_16_16_16_SInt = 0x00000415, //
        Format_16_16_16_16_Single = 0x00000515, //
        Format_16_16_16_16_UIntToSingle = 0x0000080E,
        Format_16_16_16_16_SIntToSingle = 0x00000A0E,
        // 32 bits (32 x 1)
        Format_32_UInt = 0x00000314,
        Format_32_SInt = 0x00000416,
        Format_32_Single = 0x00000516,
        // 64 bits (32 x 2)
        Format_32_32_UInt = 0x00000317, //
        Format_32_32_SInt = 0x00000417, //
        Format_32_32_Single = 0x00000517, //
        // 96 bits (32 x 3)
        Format_32_32_32_UInt = 0x00000318, //
        Format_32_32_32_SInt = 0x00000418, //
        Format_32_32_32_Single = 0x00000518, //
        // 128 bits (32 x 4)
        Format_32_32_32_32_UInt = 0x00000319, //
        Format_32_32_32_32_SInt = 0x00000419, //
        Format_32_32_32_32_Single = 0x00000519 //
    };


    struct BinaryHeader {
        ulong Magic; //MAGIC + padding

        byte VersionMicro;
        byte VersionMinor;
        ushort VersionMajor;

        ushort ByteOrder;
        byte Alignment;
        byte TargetAddressSize;
        uint NameOffset;
        ushort Flag;
        ushort BlockOffset;
        uint RelocationTableOffset;
        uint FileSize;
    };

    struct BfresHeader {

        
        ulong NameOffset;

        ulong ModelOffset;
        ulong ModelDictionaryOffset;

        ulong Reserved0;
        ulong Reserved1;
        ulong Reserved2;
        ulong Reserved3;

        ulong SkeletalAnimOffset;
        ulong SkeletalAnimDictionaryOffset;
        ulong MaterialAnimOffset;
        ulong MaterialAnimDictionarymOffset;
        ulong BoneVisAnimOffset;
        ulong BoneVisAnimDictionarymOffset;
        ulong ShapeAnimOffset;
        ulong ShapeAnimDictionarymOffset;
        ulong SceneAnimOffset;
        ulong SceneAnimDictionarymOffset;

        ulong MemoryPoolOffset;
        ulong MemoryPoolInfoOffset;

        ulong EmbeddedFilesOffset;
        ulong EmbeddedFilesDictionaryOffset;

        ulong UserPointer;

        ulong StringTableOffset;
        uint StringTableSize;

        ushort ModelCount;

        ushort Reserved4;
        ushort Reserved5;

        ushort SkeletalAnimCount;
        ushort MaterialAnimCount;
        ushort BoneVisAnimCount;
        ushort ShapeAnimCount;
        ushort SceneAnimCount;
        ushort EmbeddedFileCount;

        byte ExternalFlags;
        byte Reserved6;
        
    };

    struct BufferMemoryPool
    {
        uint Flag;
        uint Size;
        ulong Offset;

        ulong Reserved1;
        ulong Reserved2;
    };

    struct FMDL {

        char magic[4];
        byte res0[4];

        ulong nameOffset;
        ulong originalPathOffset;
        ulong fsklOffset;
        ulong fvtxOffset;
        ulong fshpOffset;
        ulong fshpDictOffset;
        ulong fmatOffset;
        byte res1[8];

        s64 fmatDictOffset;
        s64 userDataOffset;
        s64 userDictOffset;
        s64 userPointer;

        u16 fvtxCount;
        u16 fshpCount;
        u16 fmatCount;
        u16 userDataCount;

        s32 vertexCount;

        byte res2[4];

    };

    struct FSKL {
        char magic[4];

        uint flag;
        s64 boneDictOffset;
        s64 boneArrayOffset;
        s64 boneIndexOffset;
        s64 matrixOffset;
        s64 userPtr;
        s64 mirroringBoneTableOffset;
        
        u16 boneCount;
        u16 smoothBoneCount;
        u16 rigidBoneCount;
    };

    struct BufferData {
        int Stride;
        std::vector<byte> data;
    };

    struct FVTX {
        char magic[4];

        byte res0[4];

        s64 vertexAttributeArrayOffset;
        s64 vertexAttributeDictOffset;

        s64 memPoolPtr;
        s64 vbuffArrayOffset;
        s64 vbuffPtrArrayOffset;
        s64 vbuffInfoArrayOffset;
        s64 vbuffInfoStateOffset;

        s64 userPtr;
        u32 memPoolOff;
        byte vertexAttributeCount;
        byte bufferCount;
        u16 sectionIndex;
        u32 vertexCount;

        byte maxBoneInfluences;
        byte res1;
        u16 vbufferAlignment;
    };

    struct FVTXBufferInfo {
        uint Size;
    };

    struct FVTXStrideInfo {
        uint Stride;
    };
    
    struct FVTXAttribute {
        s64 nameOffset;
        uint vbFormat;
        u16 bufferOffset;
        byte bufferIndex;
        byte flag;
    };

    struct FVTXBuffer {
        byte state;
        byte flag;
        byte res[6];
        s64 nvnBuffOffset;
        byte nvnBuff[48];
        s64 userPtr;
    };
};

#endif // TESTPROJ_HPP
