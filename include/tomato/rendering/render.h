#if !defined(RENDER_H)
#define RENDER_H

#include "tmgl.h"
#include "ecs/actor.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ecs/actor.h"
#include "ecs/actor.h"
#include "ecs/actor.h"
#include "ecs/actor.h"

#include "glm/gtx/quaternion.hpp"

#include "util/filesystem_tm.h"

class tmSkinnedMeshRenderer;
class tmShader;
struct tmBaseCamera;
class tmRenderMgr;
struct tmEngine;
class tmModel;

class TMAPI tmTexture
{
public:

    enum TextureType
    {
	    IMAGE,
        FRAMEBUFFER,
        CUBE,
        SHADER
    } type;

    unsigned id, engineId;

    struct tmTextureSettings
    {
        int type = 0;

        friend bool operator==(const tmTextureSettings& lhs, const tmTextureSettings& rhs)
        {
	        return lhs.type == rhs.type;
        }

        friend bool operator!=(const tmTextureSettings& lhs, const tmTextureSettings& rhs)
        {
	        return !(lhs == rhs);
        }
    };

    tmTexture(string path, bool flip, bool appendToList = true, tmTextureSettings settings = {});
    tmTexture()
    {
        id = -1;
    }
    tmTexture(tmShader* shader, int width, int height, int format);
    ~tmTexture();

    tmTexture(vec3 c, int width, int height);

    void use(int offset);

    virtual void unload();

    static tmTexture* GetTexture(int i);
    static tmTexture* CheckTexture(string path);

    string path;
    bool flip;

private:

    inline static Dictionary<string, tmTexture*> textures = Dictionary<string, tmTexture*>();

    tmTextureSettings settings_;

    friend tmRenderMgr;

};

inline tmTexture* GetTexture(unsigned int id)
{
    return tmTexture::GetTexture(id);
}

class TMAPI tmFramebuffer : tmTexture
{
public:

    unsigned frameId = -1;
    unsigned gPos=-1, gAlb=-1, gNrm=-1, gSha=-1, gDepth=-1, gLPos = -1;
    unsigned rboDepth;
    glm::vec3 clearColor, cameraPosition;

    enum FramebufferType
    {
	    Color,
        Deferred,
        DepthOnly
    } framebuffer_type;

    tmFramebuffer(FramebufferType type, glm::vec2 size);
    ~tmFramebuffer() { unload(); }

    void use();
    void draw();

    void reload();
    void recreate(glm::vec2 size);
    void unload() override;

    glm::vec2 size;

private:
    friend tmBaseCamera;

    inline static unsigned drawVAO = -1;
    inline static class tmShader* drawShader = nullptr;


};

class TMAPI tmShader
{
public:
	unsigned id = -1;


    tmShader(string vertex, string fragment, bool isPath);
    ~tmShader();

	void reload(string vertex, string fragment, bool isPath);

	void unload()
    {

        if (id != -1 && glIsProgram(id)) {
            glDeleteProgram(id);
        }
    }

    void use();

	// utility uniform functions
// ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value)
    {
        use();
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value)
    {
        use();
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) 
    {
        use();
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value)
    {
        use();
        glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y)
    {
        use();
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value)
    {
        use();
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z)
    {
        use();
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value)
    {
        use();
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        use();
        glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat)
    {
        use();
        glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat)
    {
        use();
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat)
    {
        use();
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4Array(const std::string& name, const std::vector<glm::mat4>& mats)
    {
        if (mats.size() <= 0)
            return;

        use();
        GLint location = glGetUniformLocation(id, name.c_str());
        glUniformMatrix4fv(location, mats.size(), GL_FALSE, &mats[0][0][0]);
    }

    static List<tmShader*> shader_index;

    static tmShader* GetDefaultShader();

    bool isPath;

    string vertData;
    string fragData;

private:

    friend tmRenderMgr;


};

struct TMAPI tmBaseCamera
{
    glm::mat4 GetViewMatrix()
    {
        return view;
    }

    glm::mat4 GetProjectionMatrix()
    {
        return proj;
    }

    enum CameraClearFlags
    {
        Skybox,
        SolidColor,
        DepthOnly,
        DontClear
    } ClearFlags = Skybox;

    enum CameraProjection
    {
        Perspective,
        Orthographic
    } Projection = Perspective;

    glm::vec3 ClearColor = { 0.2f,0.3f,0.3f };
    float FieldOfView = 60, Size = 5;
    float NearPlane = EPSILON, FarPlane = 1000;
    bool EnableOcclusionCulling;
    tmFramebuffer* framebuffer;
    glm::mat4 view = glm::mat4(1.0), proj = glm::mat4(1.0), orthoProj = glm::mat4(1.0);

    void UpdateShader(tmShader* shader, bool orthoProj=false);
    void Use();


};

class TMAPI tmCamera final : public Component, public tmBaseCamera
{
	CLASS_DECLARATION(tmCamera)

public:
	tmCamera(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmCamera();
    void Deserialize(nlohmann::json j) override;

    void Start() override;
    void Update() override;
    void LateUpdate() override;
    nlohmann::json Serialize() override;
    void EngineRender() override;

    static tmCamera* GetMainCamera();

    bool doRender = true;

private:

    static tmCamera* _mainCamera;


};

const int MAX_BONE_INFLUENCE = 4;
#define MAX_BONES 252

struct tmBoneInfo
{
    /*id is index in finalBoneMatrices*/
    int id;

    /*offset matrix transforms vertex from model space to bone space*/
    glm::mat4 offset;

};

struct TMAPI tmVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];

    ~tmVertex()
    {
        //delete[] m_BoneIDs;
        //delete[] m_Weights;
    }
};

struct TMAPI tmVertexBuffer
{
    unsigned int VAO, VBO, EBO = -1, shader;

    GLenum mode=GL_TRIANGLES,elementType=GL_UNSIGNED_INT;
    int vertexCount,elementCount;

    tmVertexBuffer(tmVertex* data, size_t vertexCount,size_t elementCount, unsigned int* indices);
    tmVertexBuffer(uint vao, uint vbo, uint ebo = -1) : VAO(vao), VBO(vbo),EBO(ebo) {} ;
    tmVertexBuffer() = default;
    ~tmVertexBuffer();
};

#define fieldtype(type) void SetField(string name, type v);
#define fieldtypedecl(type) void tmMaterial::SetField(string name, type v) \
    { \
		    int i = 0; \
			for (auto shader_field : fields.GetVector()) { \
			    if (shader_field.name == name) break; \
			    i++; \
			} \
			if (i >= fields.GetCount()) \
				return; \
            fields[i].data.type##_0 = v; \
    } \

class TMAPI tmMaterial
{
public:
    unsigned materialIndex;

    tmShader* shader;

    enum tmFieldType
    {
        INT = GL_INT,
        FLOAT = GL_FLOAT,
        BOOL = GL_BOOL,

        VEC2 = GL_FLOAT_VEC2,
        VEC3 = GL_FLOAT_VEC3,
        VEC4 = GL_FLOAT_VEC4,

        MAT2 = GL_FLOAT_MAT2,
        MAT3 = GL_FLOAT_MAT3,
        MAT4 = GL_FLOAT_MAT4,

        SAMPLER2D = GL_SAMPLER_2D,
        SAMPLERCUBE = GL_SAMPLER_CUBE,

        NULLTYPE = -1,
    };

    struct tmShaderField
    {
        string name = "";
        u32 type = NULLTYPE;

        union data
        {
            int int_0;
            float float_0;
            bool bool_0;
            vec2 vec2_0;
            vec3 vec3_0;
            vec4 vec4_0;

            mat2 mat2_0;
            mat3 mat3_0;
            mat4 mat4_0;
        } data;
    };

    List<tmShaderField> fields;

    string name = "New Material";

    tmShaderField GetField(string name);
    void SetField(string name, tmShaderField field);
    void SetTexture(string name, tmTexture* texture);

    tmMaterial(tmShader* shader);

    void Reload();
    void EngineRender();

    fieldtype(int)
        fieldtype(float)
        fieldtype(bool)

        fieldtype(vec2)
        fieldtype(vec3)
        fieldtype(vec4)

        fieldtype(mat2)
        fieldtype(mat3)
        fieldtype(mat4)


        struct tmMaterialSettings
    {
        bool useOrtho = false;
    } settings;

};

class TMAPI tmMesh
{
public:
    tmVertex* vertices;
    unsigned int* indices;

    tmMesh(tmVertex* verts, size_t vertCount, size_t elemCount, unsigned int* ind);
    ~tmMesh();

    struct tmDrawCall* draw(unsigned material, glm::mat4 modelMatrix,tmActor* actor = nullptr, std::vector<mat4> boneTransforms = std::vector<mat4>());

private:

    friend tmSkinnedMeshRenderer;
    friend tmModel;

    tmModel* model;

    size_t vertexCount, elementCount;
    tmVertexBuffer buffer;
    void setup();
};

class TMAPI tmRenderer : public Component
{
	CLASS_DECLARATION(tmRenderer)

public:
	tmRenderer(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmRenderer() = default;
};

class TMAPI tmMeshRenderer : public tmRenderer
{
	CLASS_DECLARATION(tmMeshRenderer)

public:
	tmMeshRenderer(std::string&& initialValue) : tmRenderer(move(initialValue))
	{
	}

    tmMeshRenderer(tmModel* model, int meshIndex);
    void Deserialize(nlohmann::json j) override;
    tmMeshRenderer() = default;
    void EngineRender() override;

    tmMesh* mesh;
    unsigned materialIndex=0;
    nlohmann::json Serialize() override;

    void Update() override;

private:

    friend class tmSkinnedMeshRenderer;

    void Initialize(tmModel* model, int meshIndex);

    int meshIndex = -1;
    tmModel* model;

};

struct tmNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<tmNodeData> children;
};


class tmSkinnedMeshRenderer final : public tmMeshRenderer
{
	CLASS_DECLARATION_NO_CLONE(tmSkinnedMeshRenderer)

	std::shared_ptr<Component> clone(int offset) override;

public:
	tmSkinnedMeshRenderer(std::string&& initialValue) : tmMeshRenderer(move(initialValue))
	{
	}

    tmSkinnedMeshRenderer(tmModel* model, int meshIndex);
    tmSkinnedMeshRenderer() = default;

    List<int> bones;
    List<string> boneNames;

    void Start() override;
    void Update() override;

};
struct tmModelSettings
{
    float importScale = 1.0f;
};

class AssimpGLMHelpers
{
public:

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }
};


class TMAPI tmModel
{
public:
    static tmModel* LoadModel(string path, tmModelSettings settings = tmModelSettings());

    enum PrimitiveType
    {
        CUBE,
        SPHERE
    };

    static tmModel* GenerateModel(PrimitiveType type);

    List<tmMesh*> meshes;
    tmPrefab* prefab;

    std::map<string, tmBoneInfo> m_BoneInfoMap; //
    int m_BoneCounter = 0;

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

        void SetVertexBoneDataToDefault(tmVertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

        ~tmModel();

private:

    friend tmMeshRenderer;
    friend tmSkinnedMeshRenderer;
    friend tmRenderMgr;

    string path;

    tmModel(string path, tmModelSettings settings);
    tmModel();

    void loadModel(string path);
    void processNode(aiNode* node, const aiScene* scene, tmActor* parent, List<tmActor*>& bones,bool isBone, string overrideName = "");
    tmMesh* processMesh(aiMesh* mesh, const aiScene* scene);

    void SetVertexBoneData(tmVertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void ExtractBoneWeightForVertices(std::vector<tmVertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                tmBoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    tmModelSettings settings = tmModelSettings();
    aiScene* scene_;

    static Dictionary<string, tmModel*> loadedModels;
};

template <typename T>
struct tmKey
{
    T value;
    float timeStamp;
};

struct tmKeyPosition : tmKey<vec3> {};
struct tmKeyRotation : tmKey<glm::quat> {};
struct tmKeyScale : tmKey<vec3> {};

class tmBone
{
private:
    std::vector<tmKeyPosition> m_Positions;
    std::vector<tmKeyRotation> m_Rotations;
    std::vector<tmKeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;


public:

    ~tmBone();

    /*reads keyframes from aiNodeAnim*/
    tmBone(const std::string& name, int ID, const aiNodeAnim* channel)
        :
        m_Name(name),
        m_ID(ID),
        m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;

        for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        	tmKeyPosition data;
            data.value = AssimpGLMHelpers::GetGLMVec(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            tmKeyRotation data;
            data.value = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            tmKeyScale data;
            data.value = AssimpGLMHelpers::GetGLMVec(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }

    /*interpolates  b/w positions,rotations & scaling keys based on the curren time of
    the animation and prepares the local transformation matrix by combining all keys
    tranformations*/
    void Update(float animationTime)
    {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }


    /* Gets the current index on mKeyPositions to interpolate to based on
    the current animation time*/
    int GetPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_NumPositions - 1; ++index)
        {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    /* Gets the current index on mKeyRotations to interpolate to based on the
    current animation time*/
    int GetRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_NumRotations - 1; ++index)
        {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    /* Gets the current index on mKeyScalings to interpolate to based on the
    current animation time */
    int GetScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_NumScalings - 1; ++index)
        {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

private:

    /* Gets normalized value for Lerp & Slerp*/
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    /*figures out which position keys to interpolate b/w and performs the interpolation
    and returns the translation matrix*/
    glm::mat4 InterpolatePosition(float animationTime)
    {
        if (1 == m_NumPositions)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].value);

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
            m_Positions[p1Index].timeStamp, animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].value,
            m_Positions[p1Index].value, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    /*figures out which rotations keys to interpolate b/w and performs the interpolation
    and returns the rotation matrix*/
    glm::mat4 InterpolateRotation(float animationTime)
    {
        if (1 == m_NumRotations)
        {
            auto rotation = glm::normalize(m_Rotations[0].value);
            return glm::toMat4(rotation);
        }

        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
            m_Rotations[p1Index].timeStamp, animationTime);
        glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].value,
            m_Rotations[p1Index].value, scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);
    }

    /*figures out which scaling keys to interpolate b/w and performs the interpolation
    and returns the scale matrix*/
    glm::mat4 tmBone::InterpolateScaling(float animationTime)
    {
        if (1 == m_NumScalings)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].value);

        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
            m_Scales[p1Index].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0Index].value, m_Scales[p1Index].value
            , scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

};

class tmAnimation
{
public:
    tmAnimation() = default;

    tmAnimation(const std::string& animationPath, int animationindex, tmModel* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[animationindex];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        name = animation->mName.C_Str();

        if (name.empty())
        {
            name = fs::path(animationPath).filename().string() + "_" + std::to_string(animationindex);
        }

        ReadHeirarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);

        animationIndex.Add(this);
    }

    tmAnimation(const std::string& animationPath, tmModel* model);

    ~tmAnimation();

    tmBone* FindBone(const std::string& name)
    {
        auto iter = m_Bones.find(name);
        if (iter != m_Bones.end()) {
            return iter->second;
        }
        return nullptr;
    }


    inline float GetTicksPerSecond() { return m_TicksPerSecond; }

    inline float GetDuration() { return m_Duration; }

    inline const tmNodeData& GetRootNode() { return m_RootNode; }

    inline const std::map<std::string, tmBoneInfo>& GetBoneIDMap()
    {
        return m_BoneInfoMap;
    }

    string name;
    string rootName;

    static List<tmAnimation*> animationIndex;

private:
    void ReadMissingBones(const aiAnimation* animation, tmModel& model)
    {
        int size = animation->mNumChannels;

        auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
        int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones[boneName] = new tmBone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel);
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHeirarchyData(tmNodeData& dest, const aiNode* src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            tmNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
    float m_Duration;
    int m_TicksPerSecond;
    std::unordered_map<std::string, tmBone*> m_Bones;
    tmNodeData m_RootNode;
    std::map<std::string, tmBoneInfo> m_BoneInfoMap;
};

class tmSkeleton : public Component
{
	CLASS_DECLARATION(tmSkeleton)

public:
	tmSkeleton(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmSkeleton() = default;

    std::map<string, tmBoneInfo> m_BoneInfoMap; //
    u32 armatureIndex;

    void Start() override;
    void Update() override;

    List<mat4> GetBoneTransforms();

private:

    List<mat4> bone_transforms;

};

class tmAnimator : public Component
{
	CLASS_DECLARATION_NO_CLONE(tmAnimator)

    std::shared_ptr<Component> clone(int offset) override;

public:
	tmAnimator(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmAnimator() = default;

    void Start() override;
    void Update() override;
    void EngineRender() override;
    tmAnimation* currentAnimation;
    u32 skeletonIndex;

private:
    float m_CurrentTime;
    float m_DeltaTime;
    tmSkeleton* skeleton_;
};



struct TMAPI tmDrawCall
{
    unsigned material;
    glm::mat4 modelMatrix;
    std::vector<mat4> boneMats;
    tmVertexBuffer* buffer;
    unsigned drawID;
    tmActor* actor = nullptr;


    std::vector<float> subData;
    size_t subDataSize;

    Dictionary<string, tmMaterial::tmShaderField> overrideFields;

    tmDrawCall(tmVertexBuffer* buffer, unsigned materialIndex)
    {
        this->buffer = buffer;
        this->material = materialIndex;
    }

    ~tmDrawCall() = default;
};


class TMAPI tmRenderMgr
{


    void DeserializeGame(nlohmann::json j);
    nlohmann::json SerializeGame();

    friend tmCamera;
    friend tmMaterial;
    friend tmEngine;

public:

    std::vector<tmDrawCall*> drawCalls;
    std::vector<tmMaterial*> materials;

    tmRenderMgr();
    ~tmRenderMgr();

    tmDrawCall* InsertCall(tmVertexBuffer* buffer, unsigned material, std::vector<mat4> boneTransforms, glm::mat4 modelMatrix = glm::mat4(1.0),tmActor* actor = nullptr);

    void Draw();

    void Clear()
    {

        for (auto draw_call : drawCalls) delete draw_call;

        drawCalls.clear();
    }
};

#endif // RENDER_H
