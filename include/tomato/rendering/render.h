#if !defined(RENDER_H)
#define RENDER_H

#include "tmgl.h"
#include "ecs/actor.h"

class tmTexture
{
public:
    unsigned id;

    tmTexture(string path, bool flip);

    void use(int offset);
};

class tmShader
{
public:
	unsigned id;

    tmShader(string vertex, string fragment);


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


};

class Camera : public Component
{
	CLASS_DECLARATION(Camera)

public:
	Camera(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	Camera() = default;

    void Start() override;
    void Update() override;

    void UpdateShader(tmShader* shader);

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

    float FieldOfView = 60, Size = 5;
    float NearPlane=0.3f, FarPlane=1000;
    bool EnableOcclusionCulling;


private:

    glm::mat4 view=glm::mat4(1.0), proj=glm::mat4(1.0);

};

struct tmVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 texcoords;
};

struct tmVertexBuffer
{
    unsigned int VAO, VBO, EBO = -1, shader;

    GLenum mode=GL_TRIANGLES,elementType=GL_UNSIGNED_INT;
    int vertexCount;

    void Draw();

    tmVertexBuffer(tmVertex* data, size_t vertexCount, unsigned int* indices);
};

class tmMaterial
{
	
};

class tmMesh
{
public:
    List<tmVertex> vertices;
    List<unsigned int> indices;
    unsigned int material;

    tmMesh(List<tmVertex> verts, List<unsigned int> ind);
    tmMesh(tmVertex* verts, unsigned int* ind);
    tmMesh(std::vector<tmVertex> verts, std::vector<unsigned int> ind);

private:



};

class tmModel
{
	
};

#endif // RENDER_H
