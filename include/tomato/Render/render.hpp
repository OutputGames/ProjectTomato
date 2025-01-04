#ifndef RENDER_H
#define RENDER_H

#include "utils.hpp"
#include "Fs/fs.hpp"
#include "Obj/obj.hpp"


struct aiMesh;
struct aiAnimation;
struct aiNode;

namespace tmt::obj
{
    struct Object;
}

namespace tmt::obj
{
    struct CameraObject;
}

namespace tmt::light
{
    struct Light;
}

namespace tmt::render
{
    struct Animator;
    struct SkeletonObject;
    struct SceneDescription;

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
        GLFWwindow* window;
        bgfx::ViewId clearView;
        int windowWidth, windowHeight;
        bool useImgui = true;
        bool usePosAnim = true;

        static RendererInfo* GetRendererInfo();
    };

    struct ShaderInitInfo
    {
        SubShader *vertexProgram, *fragmentProgram;
        string name = "UNDEFINED";
    };

    struct ShaderUniform
    {
        bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
        string name;
        bgfx::UniformType::Enum type;

        glm::vec4 v4 = glm::vec4(0);
        glm::mat3 m3 = glm::mat3(1.0);
        glm::mat4 m4 = glm::mat4(1.0);
        Texture* tex = nullptr;

        void Use(SubShader* shader);

        ~ShaderUniform();
    };

    struct SubShader
    {
        enum ShaderType
        {
            Vertex = 0,
            Fragment,
            Compute
        } type;

        bgfx::ShaderHandle handle;
        std::vector<ShaderUniform*> uniforms;
        std::vector<string> texSets;
        string name;

        ShaderUniform* GetUniform(string name);

        void Reload();

        ~SubShader();

        static SubShader* CreateSubShader(string name, ShaderType type);

    private:
        bool isLoaded = false;

        friend fs::ResourceManager;
        SubShader(string name, ShaderType type);
    };

    struct Shader
    {
        bgfx::ProgramHandle program;
        std::vector<SubShader*> subShaders;
        string name;

        void Push(int viewId = 0, MaterialOverride** overrides = nullptr, size_t overrideCount = 0);

        ~Shader();

        void Reload();

        static Shader* CreateShader(ShaderInitInfo info);
        static Shader* CreateShader(string vertex, string fragment);

    private:
        friend fs::ResourceManager;

        Shader(ShaderInitInfo info);
    };

    struct ComputeShader
    {
        bgfx::ProgramHandle program;
        SubShader* internalShader;

        void SetUniform(string name, bgfx::UniformType::Enum type, const void* data);

        void SetMat4(string name, glm::mat4 m);

        void Run(int viewId, glm::vec3 groups = {1, 1, 1});

        ~ComputeShader();

        static ComputeShader* CreateComputeShader(SubShader* shader);

    private:
        friend fs::ResourceManager;
        ComputeShader(SubShader* shader);
    };

    struct MaterialOverride
    {
        std::string name;
        glm::vec4 v4 = glm::vec4(0);
        glm::mat3 m3 = glm::mat3(1.0);
        glm::mat4 m4 = glm::mat4(1.0);
        Texture* tex = nullptr;

        SubShader::ShaderType shaderType;
        bgfx::UniformType::Enum type;
    };

    struct MaterialState
    {
        enum DepthTest
        {
            Less         = BGFX_STATE_DEPTH_TEST_LESS,
            LessEqual    = BGFX_STATE_DEPTH_TEST_LEQUAL,
            Equal        = BGFX_STATE_DEPTH_TEST_EQUAL,
            GreaterEqual = BGFX_STATE_DEPTH_TEST_GEQUAL,
            Greater      = BGFX_STATE_DEPTH_TEST_GREATER,
            NotEqual     = BGFX_STATE_DEPTH_TEST_NOTEQUAL,
            Never        = BGFX_STATE_DEPTH_TEST_NEVER,
            Always       = BGFX_STATE_DEPTH_TEST_ALWAYS,
        } depth = Less;

        enum CullMode
        {
            Clockwise        = BGFX_STATE_CULL_CW,
            Counterclockwise = BGFX_STATE_CULL_CCW
        } cull = Counterclockwise;

        enum WriteMode
        {
            Red   = BGFX_STATE_WRITE_R,
            Green = BGFX_STATE_WRITE_G,
            Blue  = BGFX_STATE_WRITE_B,
            Alpha = BGFX_STATE_WRITE_A,
            Depth = BGFX_STATE_WRITE_Z,
            RGB   = Red | Green | Blue,
            RGBA  = RGB | Alpha,
            All   = BGFX_STATE_WRITE_MASK
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

#ifdef TMT_EDITOR

        inline static std::map<DepthTest, const char*> depthNames = {
            {
                {toName(Less)},
                {toName(LessEqual)},
                {toName(Equal)},
                {toName(GreaterEqual)},
                {toName(Greater)},
                {toName(NotEqual)},
                {toName(Never)},
                {toName(Always)},
            }
        };

        inline static std::map<CullMode, const char*> cullNames = {
            {toName(Clockwise)},
            {toName(Counterclockwise)},
        };

        inline static std::map<WriteMode, const char*> writeNames = {
            {toName(All)},
            {toName(RGBA)},
            {toName(RGB)},
            {toName(Red)},
            {toName(Green)},
            {toName(Blue)},
            {toName(Depth)},
            {toName(Alpha)},
        };
#endif
    };

    struct Material
    {
        MaterialState state;
        Shader* shader;
        std::vector<MaterialOverride*> overrides;
        string name = "Material";

        MaterialOverride* GetUniform(string name, bool force = false);
        u64 GetMaterialState();

        Material(Shader* shader = nullptr);

        void Reload(Shader* shader);

        ~Material();
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
        Vertex* vertices;
        u16* indices;

        std::vector<string> bones;
        string name;

        Model* model = nullptr;
        int idx = -1;

        ~Mesh();

        void use();

        void draw(glm::mat4 t, Material* material, std::vector<glm::mat4> anims = std::vector<glm::mat4>());
    };

    struct BoneInfo
    {
        int id;

        glm::mat4 offset;
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

            template <typename T>
            struct NodeKey
            {
                float time;
                T value;
            };

            std::vector<NodeKey<glm::vec3>*> positions;
            std::vector<NodeKey<glm::quat>*> rotations;
            std::vector<NodeKey<glm::vec3>*> scales;

        };

        std::vector<NodeChannel*> nodeChannels;

        Animation(fs::BinaryReader* reader);
        Animation() = default;

        static std::vector<Animation*> LoadAnimations(string path);

        ~Animation();

        void LoadFromAiAnimation(aiAnimation* animation);
    };

    struct Skeleton
    {
        struct Bone
        {
            struct VertexWeight
            {
                uint vertexId;
                float weight;
            };

            string name;
            glm::vec3 position{}, scale{1};
            glm::quat rotation{1, 0, 0, 0};

            glm::mat4 transformation = glm::mat4(-1);
            glm::mat4 GetTransformation();

            std::vector<string> children;

            Skeleton* skeleton;

            std::vector<VertexWeight> weights;

        };

        std::vector<Bone*> bones;
        std::map<string, BoneInfo> boneInfoMap;
        string rootName;
        glm::mat4 inverseTransform;

        Bone* GetBone(string name);

        void CalculateBoneIds();

        Skeleton(fs::BinaryReader* reader);
        Skeleton() = default;

    };

    struct Model
    {
        std::vector<Mesh*> meshes;
        std::vector<int> materialIndices;
        std::vector<Texture*> textures;
        std::vector<MaterialDescription*> materials;
        std::vector<Animation*> animations;
        Skeleton* skeleton;
        string name;

        Model(string path);
        Model(const aiScene* scene);
        Model(const aiScene* scene, SceneDescription* description);
        Model(fs::BinaryReader* reader, SceneDescription* description);

        obj::Object* CreateObject(Shader* shader = nullptr);

        Texture* GetTextureFromName(string name);

        Material* CreateMaterial(int index, Shader* shader);
        Material* CreateMaterial(MaterialDescription* materialDesc, Shader* shader);

        Animation* GetAnimation(string name);

    private:
        void LoadFromAiScene(const aiScene* scene, SceneDescription* description = nullptr);

        ~Model();
    };

    struct SceneDescription
    {
        struct Node
        {
            string name;

            glm::vec3 position{0};
            glm::quat rotation{glm::vec3{0}};
            glm::vec3 scale{1};

            std::vector<Node*> children;
            std::vector<int> meshIndices;

            Node* parent = nullptr;
            int id = -1;
            bool isBone = false;
            SceneDescription* scene = nullptr;

            void SetParent(Node* parent);
            void AddChild(Node* child);

            Node() = default;

            Node(fs::BinaryReader* reader, SceneDescription* scene);
            Node(aiNode* node, SceneDescription* scene);

            Node* GetNode(string name);
            Node* GetNode(aiNode* node);

            obj::Object* ToObject(int modelIndex = -1);

            std::vector<Node*> GetAllChildren();
        };

        string name;
        std::vector<Model*> models;
        Node* rootNode = nullptr;

        Model* GetModel(string name);
        Mesh* GetMesh(int idx);

        Node* GetNode(string name);
        Node* GetNode(aiNode* node);
        Node* GetNode(aiMesh* mesh, int index);

        std::vector<Node*> GetAllChildren();

        SceneDescription(string path);

        obj::Object* ToObject();

        ~SceneDescription();
    };


    struct BoneObject : obj::Object
    {
        Skeleton::Bone* bone;
        BoneObject* copyBone;

        int Load(SceneDescription::Node* node, int count);

        glm::mat4 GetGlobalOffsetMatrix();
        glm::mat4 GetOffsetMatrix();

        void Update() override;

        void CalculateBoneMatrix(SkeletonObject* skeleton, glm::mat4 parentMatrix);
    };

    struct SkeletonObject : obj::Object
    {
        std::vector<BoneObject*> bones;

        std::vector<glm::mat4> boneMatrices;
        Animator* animator;

        Skeleton* skeleton;

        void Update() override;

        void Load(SceneDescription::Node* node);

        BoneObject* GetBone(string name);

        bool IsSkeletonBone(BoneObject* bone);

        void CalculateBoneTransform(const Skeleton::Bone* skeleBone, glm::mat4 parentTransform);
    };

    struct Animator : obj::Object
    {
        std::vector<glm::mat4> pushBoneMatrices;

        Animator();

        struct AnimationBone
        {
            Animation::NodeChannel* channel = nullptr;
            Animation* animation = nullptr;
            glm::mat4 localTransform;
            int boneId;

            glm::mat4 Update(float animationTime, Object* obj);

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

        AnimationBone* GetBone(string name);

        float time = 0;

        void Update() override;

        void LoadAnimationBones();

        void SetAnimation(Animation* animation);

        void CalculateBoneTransform(const Skeleton::Bone* skeleBone, glm::mat4 parentTransform);;

        Animation* currentAnimation = nullptr;

    };


    struct Texture
    {
        string name;
        bgfx::TextureHandle handle;
        bgfx::TextureFormat::Enum format;

        int width, height;

        static Texture* CreateTexture(string path, bool isCubemap = false);

        Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags = 0, const bgfx::Memory* mem = nullptr,
                string name = "");
        ~Texture();

    private:
        Texture(string path, bool isCubemap = false);
    };

    struct RenderTexture
    {
        bgfx::FrameBufferHandle handle;
        bgfx::ViewId vid = 1;
        Texture* realTexture;
        Texture* depthTexture;

        bgfx::TextureFormat::Enum format;

        RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 clearFlags);

        ~RenderTexture();
    };

    struct Camera
    {
        glm::vec3 position;
        glm::quat rotation;
        float FOV = 90.0f;
        float NearPlane = 0.001f;
        float FarPlane = 1000.0f;

        float* GetView();
        const float* GetProjection();

        glm::mat4 GetView_m4();
        glm::mat4 GetProjection_m4();

        glm::vec3 GetFront();
        glm::vec3 GetUp();

        static Camera* GetMainCamera();

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

        MaterialOverride** overrides;
        size_t overrideCt = 0;

        Mesh* mesh;
        glm::mat4 transformMatrix;
        std::vector<glm::mat4> animationMatrices;
        //float animationMatrices[4][4][MAX_BONE_MATRICES];
        int matrixCount = 0;
        Shader* program;
        MaterialState::MatrixMode matrixMode;
    };

    MatrixArray GetMatrixArray(glm::mat4 m);

    Mesh* createMesh(Vertex* data, u16* indices, u32 vertSize, u32 triSize, bgfx::VertexLayout pcvDecl,
                     Model* model = nullptr, string name = "none");

    void pushDrawCall(DrawCall d);

    void pushLight(light::Light* light);

    RendererInfo* init();

    void update();

    void shutdown();

}

namespace bgfx
{
    void setUniform(UniformHandle handle, glm::vec4 v);
    void setUniform(UniformHandle handle, std::vector<glm::vec4> v);
    void setUniform(UniformHandle handle, std::vector<glm::mat4> v);
} // namespace bgfx

#endif
