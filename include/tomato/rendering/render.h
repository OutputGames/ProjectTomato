#if !defined(RENDER_H)
#define RENDER_H

#include "tmgl.h"
#include "ecs/actor.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class tmRenderMgr;
struct tmEngine;
class tmModel;

class TMAPI tmTexture
{
public:
    unsigned id;

    tmTexture(string path, bool flip);
    tmTexture()
    {
        id = -1;
    }

    void use(int offset);

private:

    friend tmRenderMgr;

    string path;
    bool flip;

};

class TMAPI tmFramebuffer : public tmTexture
{
public:

    unsigned frameId = -1;
    unsigned gPos=-1, gAlb=-1, gNrm=-1, gSha=-1;
    unsigned rboDepth;
    glm::vec3 clearColor;

    enum FramebufferType
    {
	    Color,
        Deferred
    } type;

    tmFramebuffer(FramebufferType type, glm::vec2 size);

    void use();
    void draw();

    void reload();
    void recreate(glm::vec2 size);
    void unload();

    glm::vec2 size;

private:
    inline static unsigned drawVAO = -1;
    inline static class tmShader* drawShader = nullptr;
};

class TMAPI tmShader
{
public:
	unsigned id;

    tmShader(string vertex, string fragment);

    void reload(string vertex, string fragment)
    {
        unload();
        id = tmgl::genShader(vertex.c_str(), fragment.c_str());
    }

    void unload()
    {
        glDeleteShader(id);
    }

    void use()
    {
        glUseProgram(id);
    }

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

private:

    friend tmRenderMgr;

    string vertData;
    string fragData;
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
    float NearPlane = 0.3f, FarPlane = 1000;
    bool EnableOcclusionCulling;
    tmFramebuffer* framebuffer;
    glm::mat4 view = glm::mat4(1.0), proj = glm::mat4(1.0);

    void UpdateShader(tmShader* shader);
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

    static tmCamera* GetMainCamera();

private:

    static tmCamera* _mainCamera;


};

struct TMAPI tmVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};

struct TMAPI tmVertexBuffer
{
    unsigned int VAO, VBO, EBO = -1, shader;

    GLenum mode=GL_TRIANGLES,elementType=GL_UNSIGNED_INT;
    int vertexCount,elementCount;

    tmVertexBuffer(tmVertex* data, size_t vertexCount,size_t elementCount, unsigned int* indices);
    tmVertexBuffer() = default;
};

class TMAPI tmMaterial
{
public:
    unsigned materialIndex;

    tmShader* shader;

    std::vector<tmTexture*> textures;

    tmMaterial(tmShader* shader);
};

class TMAPI tmMesh
{
public:
    tmVertex* vertices;
    unsigned int* indices;

    tmMesh(tmVertex* verts, size_t vertCount, size_t elemCount, unsigned int* ind);

    struct tmDrawCall* draw(unsigned material, glm::mat4 modelMatrix);

private:
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

    tmMesh* mesh;
    unsigned materialIndex=-1;
    nlohmann::json Serialize() override;

    void Update() override;

private:

    void Initialize(tmModel* model, int meshIndex);

    int meshIndex = -1;
    tmModel* model;

};

class TMAPI tmModel
{
public:
    static tmModel* LoadModel(string path);

    List<tmMesh*> meshes;

private:

    friend tmMeshRenderer;

    string path;

    tmModel(string path);

    void loadModel(string path);
    void processNode(aiNode* node, const aiScene* scene);
    tmMesh* processMesh(aiMesh* mesh, const aiScene* scene);

    static Dictionary<string, tmModel*> loadedModels;
};

struct TMAPI tmDrawCall
{
    unsigned material;
    glm::mat4 modelMatrix;
    tmVertexBuffer* buffer;

    tmDrawCall(tmVertexBuffer* buffer, unsigned materialIndex)
    {
        this->buffer = buffer;
        this->material = materialIndex;
    }
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

    tmRenderMgr() = default;

    tmDrawCall* InsertCall(tmVertexBuffer* buffer, unsigned material,glm::mat4 modelMatrix = glm::mat4(1.0));

    void Draw();

    void Clear()
    {

        for (auto draw_call : drawCalls) delete draw_call;

        drawCalls.clear();
    }
};

#endif // RENDER_H
