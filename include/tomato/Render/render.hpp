#ifndef RENDER_H
#define RENDER_H

#include <complex.h>

#include "utils.hpp"
#include "Fs/fs.hpp"
#include "Obj/obj.hpp"


struct aiTexel;
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
        //tmgl::ViewId clearView;
        int windowWidth, windowHeight;
        bool useImgui = true;
        bool usePosAnim = true;
        std::vector<RenderTexture*> viewCache;
        std::vector<Camera*> cameraCache;

        static RendererInfo* GetRendererInfo();
    };

    struct ShaderInitInfo
    {
        SubShader *vertexProgram, *fragmentProgram;
        string name = "UNDEFINED";
    };

    struct ShaderUniform
    {
        tmgl::UniformHandle handle = TMGL_INVALID_HANDLE;
        string name;
        tmgl::UniformType::Enum type = tmgl::UniformType::Count;

        glm::vec4 v4 = glm::vec4(-1000);
        glm::mat3 m3 = glm::mat3(-1000.0);
        glm::mat4 m4 = glm::mat4(-1000.0);
        Texture* tex = nullptr;
        int forcedSamplerIndex = -1;

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

        tmgl::ShaderHandle handle;
        std::vector<ShaderUniform*> uniforms;
        std::vector<string> texSets;
        string name;

        ShaderUniform* GetUniform(string name, bool force = false);

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
        tmgl::ProgramHandle program;
        std::vector<SubShader*> subShaders;
        string name;

        void Push(int viewId = 0, MaterialOverride* overrides = nullptr, size_t overrideCount = 0);

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
        tmgl::ProgramHandle program;
        SubShader* internalShader;

        void SetUniform(string name, tmgl::UniformType::Enum type, const void* data);

        void SetMat4(string name, glm::mat4 m);
        void SetVec4(string name, glm::vec4 v);

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

        int forcedSamplerIndex = -1;

        SubShader::ShaderType shaderType = SubShader::Fragment;
        tmgl::UniformType::Enum type;
    };

    struct MaterialState
    {
        enum DepthTest
        {
            Less         = TMGL_STATE_DEPTH_TEST_LESS,
            LessEqual    = TMGL_STATE_DEPTH_TEST_LEQUAL,
            Equal        = TMGL_STATE_DEPTH_TEST_EQUAL,
            GreaterEqual = TMGL_STATE_DEPTH_TEST_GEQUAL,
            Greater      = TMGL_STATE_DEPTH_TEST_GREATER,
            NotEqual     = TMGL_STATE_DEPTH_TEST_NOTEQUAL,
            Never        = TMGL_STATE_DEPTH_TEST_NEVER,
            Always       = TMGL_STATE_DEPTH_TEST_ALWAYS,
        } depth = LessEqual;

        enum CullMode
        {
            Clockwise        = TMGL_STATE_CULL_CW,
            Counterclockwise = TMGL_STATE_CULL_CCW
        } cull = Counterclockwise;

        bool writeR = true;
        bool writeG = true;
        bool writeB = true;
        bool writeA = true;

        bool writeZ = true;

        u64 srcAlpha = TMGL_STATE_BLEND_SRC_ALPHA;
        u64 dstAlpha = TMGL_STATE_BLEND_INV_SRC_ALPHA;

        void SetWrite(u64 flag);

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
        std::vector<MaterialOverride> overrides;
        string name = "Material";

        MaterialOverride* GetUniform(string name, bool force = true);
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

    enum MeshOrigin
    {
        mo_loaded,
        mo_generated
    };

    struct Mesh
    {
        tmgl::IndexBufferHandle ibh;
        tmgl::VertexBufferHandle vbh;
        std::vector<tmgl::DynamicVertexBufferHandle> vertexBuffers;
        tmgl::DynamicIndexBufferHandle indexBuffer;
        size_t vertexCount, indexCount;
        Vertex* vertices;
        u16* indices;
        MeshOrigin origin;

        std::vector<string> bones;
        string name;

        Model* model = nullptr;
        int idx = -1;

        ~Mesh();

        void use();

        virtual void draw(glm::mat4 t, Material* material, glm::vec3 spos, u32 layer = 0, u32 renderLayer = 0,
                          std::vector<glm::mat4> anims = std::vector<glm::mat4>());
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

            glm::mat4 GetWorldTransform();

            void CopyTransformation(glm::mat4 m);

            std::vector<string> children;

            Skeleton* skeleton;

            std::vector<VertexWeight> weights;

        };

        std::vector<Bone*> bones;
        std::map<string, BoneInfo> boneInfoMap;
        string rootName;
        glm::mat4 inverseTransform;

        Bone* GetBone(string name);
        Bone* GetParent(Bone* b);

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
        int GetAnimationIndex(string name);
        MaterialDescription* GetMaterial(string name);

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
            glm::vec3 GetGlobalPosition();

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

            Node* GetNode(string name, bool isPath = false);
            Node* GetNode(aiNode* node);

            obj::Object* ToObject(int modelIndex = -1);

            std::vector<Node*> GetAllChildren();
        };

        string name, path;
        std::vector<Model*> models;
        Node* rootNode = nullptr;

        Model* GetModel(string name);
        Mesh* GetMesh(int idx);

        Node* GetNode(string name, bool isPath = false);
        Node* GetNode(aiNode* node);
        Node* GetNode(aiMesh* mesh, int index);

        std::vector<Node*> GetAllChildren();

        obj::Object* ToObject();

        ~SceneDescription();

        static SceneDescription* CreateSceneDescription(string path);

    private:
        SceneDescription(string path);
    };


    struct BoneObject : obj::Object
    {
        BoneObject* copyBone = nullptr;

        BoneObject() = default;
        BoneObject(Skeleton::Bone* bone);
        void Load(SceneDescription::Node* node);

        glm::mat4 GetGlobalOffsetMatrix();
        glm::mat4 GetOffsetMatrix();

        void Update() override;

        Skeleton::Bone* bone;
    };

    struct SkeletonObject : obj::Object
    {
        std::vector<BoneObject*> bones;

        std::vector<glm::mat4> boneMatrices;
        Animator* animator;

        Skeleton* skeleton;

        SkeletonObject() = default;
        SkeletonObject(Skeleton* skl);

        void Start() override;

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

        bool doLoop = true;

        void Update() override;

        void LoadAnimationBones();

        void SetAnimation(Animation* animation);

        Animation* currentAnimation = nullptr;

    };


    struct Texture
    {
        string name;
        tmgl::TextureHandle handle;
        tmgl::TextureFormat::Enum format;

        int width, height;

        static Texture* CreateTexture(string path, u64 textureFlags = 0);

        Texture(int width, int height, tmgl::TextureFormat::Enum tf, u64 flags = 0, const tmgl::Memory* mem = nullptr,
                string name = "");

        Texture(tmgl::TextureHandle handle);
        Texture(aiTexel* texels, int width, u64 flags = 0);

        ~Texture();

    private:
        friend struct TextureAtlas;
        Texture(string path, u64 flags = 0);
        Texture();
    };


    struct TextureAtlas : Texture
    {
        static TextureAtlas* CreateTexture(string path);
        static TextureAtlas* CreateTexture(std::vector<string> paths);

        ~TextureAtlas();

    private:
        TextureAtlas(string path);
        TextureAtlas(std::vector<string> paths);
    };

    struct RenderTexture
    {
        tmgl::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
        //tmgl::ViewId vid = 1;
        Texture* realTexture = nullptr;
        Texture* depthTexture = nullptr;
        u16 viewId;

        string name = "";

        tmgl::TextureFormat::Enum format, depthFormat;

        RenderTexture(u16 width, u16 height, tmgl::TextureFormat::Enum format, u16 clearFlags);
        RenderTexture();

        ~RenderTexture();

        void resize(int width, int height);

    private:
        friend Camera;


    };

    struct CubemapTexture
    {

        CubemapTexture(string path);
    };

    struct Font
    {
        struct Character
        {
            glm::ivec2 size;
            glm::ivec2 bearing;
            unsigned int advance;
            Texture* handle;
            tmgl::VertexBufferHandle vbh;
        };

        std::map<char, Character> characters;

        static Font* Create(string path);

        float spacing = 1.0;
        tmgl::IndexBufferHandle ibh;

        float CalculateTextSize(string text, float fontSize, float forcedSpacing = FLT_MAX);

    private:
        Font(string path);

    };

    struct Camera
    {
        glm::vec3 position = {0, 0, 0};
        glm::quat rotation = {1, 0, 0, 0};

        RenderTexture* renderTexture;
        u32 renderLayers = -1;

        float FOV = 90.0f;
        float NearPlane = 0.001f;
        float FarPlane = 1000.0f;

        enum CameraMode
        {
            Perspective,
            Orthographic
        } mode = Perspective;

        float* GetView();
        const float* GetProjection();

        glm::mat4 GetView_m4();
        glm::mat4 GetProjection_m4();
        glm::mat4 GetOrthoProjection_m4();

        glm::vec3 GetFront();
        glm::vec3 GetUp();

        void redraw();

        static Camera* GetMainCamera();

    private:
        friend obj::CameraObject;
        friend obj::Scene;

        Camera();
        ~Camera();
    };

    struct Color
    {
        float r = 1, g = 1, b = 1, a = 1;

        Color(float r = 1, float g = 1, float b = 1, float a = 1)
        {
            if (r > 1)
                r /= 255;
            if (g > 1)
                g /= 255;
            if (b > 1)
                b /= 255;
            if (a > 1)
                a /= 255;

            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }

        Color(double s)
        {

            r = s;
            g = s;
            b = s;

            if (r > 1)
                r /= 255;
            if (g > 1)
                g /= 255;
            if (b > 1)
                b /= 255;
            if (a > 1)
                a /= 255;
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
            return (red << 24) | (green << 16) | (blue << 8) | alpha;
        }

        static Color FromHex(string hex);

        static Color White, Red, Blue, Green, Black, Gray;
    };

    using MatrixArray = std::vector<float>;

    struct DrawCall
    {
        u64 state;
        u64 layer = 0;

        u32 renderLayer;

        MaterialOverride* overrides;
        size_t overrideCt = 0;

        Mesh* mesh;

        tmgl::VertexBufferHandle vbh;
        tmgl::IndexBufferHandle ibh;

        u32 vertexCount, indexCount;

        glm::vec3 sortedPosition = glm::vec3(0);
        glm::mat4 transformMatrix;
        std::vector<glm::mat4> animationMatrices;
        //float animationMatrices[4][4][MAX_BONE_MATRICES];
        int matrixCount = 0;
        Shader* program;
        MaterialState::MatrixMode matrixMode;

        float getDistance(Camera* cam);
        void clean();
    };


    MatrixArray GetMatrixArray(glm::mat4 m);

    Mesh* createMesh(Vertex* data, u16* indices, u32 vertSize, u32 triSize, tmgl::VertexLayout pcvDecl,
                     Model* model = nullptr, string name = "none");

    void pushDrawCall(DrawCall d);

    void takeScreenshot(string path = "null");

    void pushLight(light::Light* light);

    RendererInfo* init(int width, int height);

    void update();

    void shutdown();

}

namespace tmgl
{
    void setUniform(UniformHandle handle, glm::vec4 v);
    void setUniform(UniformHandle handle, std::vector<glm::vec4> v);
    void setUniform(UniformHandle handle, std::vector<glm::mat4> v);;
} // namespace bgfx

#endif
