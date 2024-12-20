#ifndef RENDER_H
#define RENDER_H

#include "utils.hpp" 
#include "Fs/fs.hpp"
#include "Obj/obj.hpp"


struct aiNode;

namespace tmt::obj
{
    struct Object;
}

namespace tmt::obj {
 struct CameraObject;
 }


namespace tmt::render {

    struct RendererInfo;
struct ShaderInitInfo;
struct ShaderUniform;
struct SubShader;
struct Shader;
struct ComputeShader;
struct MaterialOverride;
struct MaterialState;
struct Material;
struct Mesh;
struct Model;
struct Texture;
struct RenderTexture;
struct Camera;
struct Color;
struct DrawCall;

struct RendererInfo
{
    GLFWwindow *window;
    bgfx::ViewId clearView;
    int windowWidth, windowHeight;
};

struct ShaderInitInfo
{
    SubShader *vertexProgram, *fragmentProgram;
};

struct ShaderUniform
{
    bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
    string name;
    bgfx::UniformType::Enum type;

    glm::vec4 v4 = glm::vec4(0);
    glm::mat3 m3 = glm::mat3(1.0);
    glm::mat4 m4 = glm::mat4(1.0);
    Texture *tex = nullptr;

    void Use();
};

struct SubShader
{
    enum ShaderType
    {
        Vertex = 0,
        Fragment,
        Compute
    };

    bgfx::ShaderHandle handle;
    std::vector<ShaderUniform *> uniforms;

    SubShader(string name, ShaderType type);

    ShaderUniform *GetUniform(string name);
};

struct Shader
{
    bgfx::ProgramHandle program;
    std::vector<SubShader *> subShaders;

    Shader(ShaderInitInfo info);

    void Push(int viewId = 0, MaterialOverride **overrides = nullptr, size_t overrideCount = 0);
};

struct ComputeShader
{
    bgfx::ProgramHandle program;
    SubShader *internalShader;

    ComputeShader(SubShader *shader);

    void SetUniform(string name, bgfx::UniformType::Enum type, const void *data);

    void SetMat4(string name, glm::mat4 m);

    void Run(int viewId, glm::vec3 groups = {1, 1, 1});
};

struct MaterialOverride
{
    std::string name;
    glm::vec4 v4 = glm::vec4(0);
    glm::mat3 m3 = glm::mat3(1.0);
    glm::mat4 m4 = glm::mat4(1.0);
    Texture *tex = nullptr;
};

struct MaterialState
{
    enum DepthTest
    {
        Less = BGFX_STATE_DEPTH_TEST_LESS,
        LessEqual = BGFX_STATE_DEPTH_TEST_LEQUAL,
        Equal = BGFX_STATE_DEPTH_TEST_EQUAL,
        GreaterEqual = BGFX_STATE_DEPTH_TEST_GEQUAL,
        Greater = BGFX_STATE_DEPTH_TEST_GREATER,
        NotEqual = BGFX_STATE_DEPTH_TEST_NOTEQUAL,
        Never = BGFX_STATE_DEPTH_TEST_NEVER,
        Always = BGFX_STATE_DEPTH_TEST_ALWAYS,
    } depth = Less;

    enum CullMode
    {
        Clockwise = BGFX_STATE_CULL_CW,
        Counterclockwise = BGFX_STATE_CULL_CCW
    } cull = Counterclockwise;

    enum WriteMode
    {
        Red = BGFX_STATE_WRITE_R,
        Green = BGFX_STATE_WRITE_G,
        Blue = BGFX_STATE_WRITE_B,
        Alpha = BGFX_STATE_WRITE_A,
        Depth = BGFX_STATE_WRITE_Z,
        All = BGFX_STATE_WRITE_MASK
    };

    u64 write = All;

    enum MatrixMode
    {
        ViewProj,
        View,
        Proj,
        ViewOrthoProj,
        OrthoProj,
        None
    } matrixMode = ViewProj;
};

struct Material
{
    MaterialState state;
    Shader *shader;
    std::vector<MaterialOverride *> overrides;

    MaterialOverride *GetUniform(string name, bool force = false);
    u64 GetMaterialState();

    Material(Shader *shader = nullptr);

    void Reload(Shader *shader);
};

    struct MaterialDescription
{
    string Name;
    std::map<string, string> Textures;
};

struct Mesh
{
    bgfx::IndexBufferHandle ibh;
    bgfx::VertexBufferHandle vbh;
    std::vector<bgfx::DynamicVertexBufferHandle> vertexBuffers;
    bgfx::DynamicIndexBufferHandle indexBuffer;
    size_t vertexCount, indexCount;
    Vertex *vertices;
    u16 *indices;
    Model* model = nullptr;
    int idx = -1;

    void draw(glm::mat4 t, Material* material, std::vector<glm::mat4> anims = std::vector<glm::mat4>());
};

    struct Animation
    {
        string name;

        float duration;
        int ticksPerSecond;

        struct NodeChannel
        {
            string name;
            int id = -1;

            template<typename T>
            struct NodeKey
            {
                float time;
                T value;
            };

            std::vector<NodeKey<glm::vec3>> positions;
            std::vector<NodeKey<glm::quat>> rotations;
            std::vector<NodeKey<glm::vec3>> scales;

        };
        std::vector<NodeChannel*> nodeChannels;

        Animation(fs::BinaryReader* reader);
        Animation() = default;
    };

    struct Skeleton
    {
        struct Bone
        {
            string name;
            int id;
            glm::vec3 position, scale;
            glm::quat rotation;
        };

        std::vector<Bone*> bones;
        string rootName;

        Skeleton(fs::BinaryReader* reader);
    };

    struct Model
{
    std::vector<Mesh *> meshes;
    std::vector<int> materialIndices;
    std::vector<Texture*> textures;
    std::vector<MaterialDescription*> materials;
    std::vector<Animation*> animations;
    Skeleton* skeleton;

    Model(string path);
    Model(const aiScene *scene);
    Model(fs::BinaryReader* reader);

    tmt::obj::Object* CreateObject(Shader*shader=nullptr);

    Texture* GetTextureFromName(string name);

        Material* CreateMaterial(int index, Shader* shader);
    Material* CreateMaterial(MaterialDescription* materialDesc, Shader* shader);

    private:
    void LoadFromAiScene(const aiScene* scene);
    
};

    struct SceneDescription
    {
        struct Node
        {
            string name;

            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;

            std::vector<Node*> children;
            std::vector<int> meshIndices;

            Node* parent = nullptr;
            int id;
            bool isBone;
            SceneDescription* scene;

            Node() = default;

            Node(fs::BinaryReader* reader, SceneDescription* scene);
            Node(aiNode* node, SceneDescription* scene);

            tmt::obj::Object* ToObject();
        };

        string name;
        std::vector<Model*> models;
        Node* rootNode;

        Mesh* GetMesh(int idx);

        SceneDescription(string path);

        obj::Object* ToObject();
    };

    
    struct BoneObject : tmt::obj::Object
    {
        int id = -1;
        int Load(SceneDescription::Node* node, int count);
    };

    struct SkeletonObject : tmt::obj::Object
    {
        std::vector<BoneObject*> bones;

        std::vector<glm::mat4> boneMatrices;

        void Update() override;

        void Load(SceneDescription::Node* node);

        BoneObject* GetBone(string name);
    };

    struct Animator : tmt::obj::Object
    {

        struct AnimationBone
        {

            Animation* animation = nullptr;
            Animation::NodeChannel* channel = nullptr;

            std::tuple<glm::vec3, glm::quat, glm::vec3> Update(float animationTime);

            int GetPositionIndex(float animationTime);
            int GetRotationIndex(float animationTime);
            int GetScaleIndex(float animationTime);

            float GetScaleFactor(float lasttime, float nexttime, float animationTime);


            glm::vec3 InterpolatePosition(float animationTime);
            glm::quat InterpolateRotation(float animationTime);
            glm::vec3 InterpolateScaling(float animationTime);
        };

        SkeletonObject* skeleton = nullptr;

        std::vector<AnimationBone*> animationBones;

        Animation* currentAnimation = nullptr;
        float time = 0;

        void Update() override;

        void LoadAnimationBones();

    };
 

    struct Texture
{
    string name;
    bgfx::TextureHandle handle;
    bgfx::TextureFormat::Enum format;

    int width, height;

    Texture(string path);
    Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags = 0, const bgfx::Memory* mem = nullptr,
            string name = "");
};

struct RenderTexture
{
    bgfx::FrameBufferHandle handle;
    bgfx::ViewId vid = 1;
    Texture *realTexture;

    bgfx::TextureFormat::Enum format;

    RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 clearFlags);
};

struct Camera
{
    glm::vec3 position, rotation;
    float FOV = 90.0f;

    float *GetView();
    float *GetProjection();

    glm::vec3 GetFront();
    glm::vec3 GetUp();

    static Camera *GetMainCamera();

  private:
    friend obj::CameraObject;

    Camera();
};

struct Color
{
    float r = 1, g = 1, b = 1, a = 1;

    Color(float r = 1, float g = 1, float b = 1, float a = 1)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    glm::vec4 getData() const
    {
        return glm::vec4{r, g, b, a};
    }

    void darken(float amt)
    {
        r += amt;
        g += amt;
        b += amt;
    }

    u32 getHex() const
    {
        // Clamp values between 0 and 255 after scaling
        uint32_t red = static_cast<uint32_t>(r * 255) & 0xFF;
        uint32_t green = static_cast<uint32_t>(g * 255) & 0xFF;
        uint32_t blue = static_cast<uint32_t>(b * 255) & 0xFF;
        uint32_t alpha = static_cast<uint32_t>(a * 255) & 0xFF;

        // Combine into a single 32-bit integer in RGBA order
        return (alpha << 24) | (blue << 16) | (green << 8) | red;
    }

    static Color White, Red, Blue, Green;
};

    using MatrixArray = std::vector<float>;

struct DrawCall
{
    u64 state;

    MaterialOverride **overrides;
    size_t overrideCt = 0;

    Mesh *mesh;
    float transformMatrix[4][4];
    std::vector<glm::mat4> animationMatrices;
    //float animationMatrices[4][4][MAX_BONE_MATRICES];
    int matrixCount = 0;
    Shader *program;
    MaterialState::MatrixMode matrixMode;
};

    MatrixArray GetMatrixArray(glm::mat4 m);

Mesh *createMesh(Vertex *data, u16 *indices, u32 vertSize, u32 triSize, bgfx::VertexLayout pcvDecl, Model* model=nullptr);

void pushDrawCall(DrawCall d);

RendererInfo *init();

void update();

void shutdown();
;

}

#endif