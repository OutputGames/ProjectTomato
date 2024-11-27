#if !defined(TOMATO_HPP)
#define TOMATO_HPP

#include <stdio.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3.h>
#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>
#include "utils.hpp"

namespace tmt {

    namespace render
    {
	    struct Texture;
	    struct MaterialOverride;
	    struct Mesh;

        struct Vertex
        {
            glm::vec3 position = glm::vec3(0.0);
            glm::vec3 normal = glm::vec3(0);
            glm::vec2 uv0 = glm::vec2(0.5);

            static bgfx::VertexLayout getVertexLayout();
        };

	    struct RendererInfo {
            GLFWwindow* window;
            bgfx::ViewId clearView;
            int windowWidth, windowHeight;
        };

        struct Shader;

        struct DrawCall
        {
            u64 state;

            MaterialOverride** overrides;
            size_t overrideCt = 0;

            Mesh* mesh;
            float transformMatrix[4][4];
            Shader* program;
        };

        struct SubShader;

        struct ShaderInitInfo
        {
            SubShader* vertexProgram, *fragmentProgram;
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
            std::vector<ShaderUniform*> uniforms;

            SubShader(string name, ShaderType type);

            ShaderUniform* GetUniform(string name);
        };

        struct Shader
        {

            bgfx::ProgramHandle program;
            std::vector<SubShader*> subShaders;


            Shader(ShaderInitInfo info);

            void Push(MaterialOverride** overrides = nullptr, size_t overrideCount=0);
        };

        struct MaterialOverride
        {

            std::string name;
            glm::vec4 v4 = glm::vec4(0);
            glm::mat3 m3 = glm::mat3(1.0);
            glm::mat4 m4 = glm::mat4(1.0);
            Texture* tex = nullptr;

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
                Alway = BGFX_STATE_DEPTH_TEST_ALWAYS,
            } depth = Less;

            enum CullMode
            {
	            Clockwise = BGFX_STATE_CULL_CW,
                Counterclockwise = BGFX_STATE_CULL_CCW
            } cull = Clockwise;

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

        };

        struct Material
        {
            MaterialState state;
            Shader* shader;
            std::vector<MaterialOverride*> overrides;

            MaterialOverride* GetUniform(string name);
            u64 GetMaterialState();

            Material(Shader* shader);


        };

        struct Mesh
        {
            bgfx::IndexBufferHandle ibh;
            bgfx::VertexBufferHandle vbh;

            void draw(glm::mat4 t, Material* material);
        };

        struct Texture {
            bgfx::TextureHandle handle;

            Texture(string path);
        };

        Mesh* createMesh(Vertex* data, u16* indices, u32 vertSize, u32 triSize, bgfx::VertexLayout pcvDecl );
        void pushDrawCall(DrawCall d);

        RendererInfo* init();
        void update();
        void shutdown();
    } // namespace engine

    namespace engine
    {
        struct EngineInfo
        {
            render::RendererInfo* renderer;
        };

        EngineInfo* init();

    }

    namespace math
    {

        float* vec4toArray(glm::vec4 v);
        float** mat4ToArray(glm::mat4 m);
        float** mat3ToArray(glm::mat3 m);


    }

    namespace obj
    {

        struct Object
        {
        private:
            virtual string GetDefaultName()
            {
                return "Object";
            }
        public:
            glm::vec3 position, rotation, scale = glm::vec3(1.0);
            string name = GetDefaultName();

            virtual void Update();

            Object* GetParent()
            {
                return parent;
            }

            void SetParent(Object* object)
            {
	            if (parent)
	            {
                    children.erase(std::find(children.begin(), children.end(), this));
                    parent = nullptr;
	            }

                parent = object;
                if (parent) {
                    parent->children.push_back(this);
                }
            }

            std::vector<Object*> GetChildren()
            {
                return children;
            }

            glm::vec3 GetGlobalPosition();
            glm::vec3 GetGlobalRotation();
            glm::vec3 GetGlobalScale();

            glm::mat4 GetTransform();

        private:
            Object* parent = nullptr;
            std::vector<Object*> children;
        };

        struct MeshObject : Object
        {


            render::Material* material;
            render::Mesh* mesh;

            void Update() override;

        private:
            string GetDefaultName() override
            {
                return "MeshObject";
            }
        };
  
    }
    

}

#endif // TOMATO_HPP
