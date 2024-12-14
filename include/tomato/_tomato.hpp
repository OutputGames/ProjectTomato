#if !defined(TOMATO_HPP)
#define TOMATO_HPP

#include "utils.hpp"

namespace tmt
{
	namespace particle
	{
		struct Particle;
	}

	namespace physics
	{
		struct ColliderObject;
		struct RaycastHit;
		struct PhysicalWorld;
	}

	namespace obj
	{
		struct CameraObject;
	}

	namespace render
	{
		struct Texture;
		struct MaterialOverride;
		struct Mesh;

		struct RendererInfo
		{
			GLFWwindow* window;
			bgfx::ViewId clearView;
			int windowWidth, windowHeight;
		};

		struct Shader;

		struct SubShader;

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

			void Push(int viewId = 0, MaterialOverride** overrides = nullptr, size_t overrideCount = 0);
		};

		struct ComputeShader
		{
			bgfx::ProgramHandle program;
			SubShader* internalShader;

			ComputeShader(SubShader* shader);

			void SetUniform(string name, bgfx::UniformType::Enum type, const void* data);

			void SetMat4(string name, glm::mat4 m);

			void Run(int viewId, glm::vec3 groups = {1, 1, 1});
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
			Shader* shader;
			std::vector<MaterialOverride*> overrides;

			MaterialOverride* GetUniform(string name);
			u64 GetMaterialState();

			Material(Shader* shader = nullptr);

			void Reload(Shader* shader);
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

			void draw(glm::mat4 t, Material* material);
		};

		struct Model
		{
			std::vector<Mesh*> meshes;
			std::vector<int> materialIndices;

			Model(string path);
			Model(const aiScene* scene);
		};

		struct Texture
		{
			bgfx::TextureHandle handle;
			bgfx::TextureFormat::Enum format;

			int width, height;

			Texture(string path);
			Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory* mem = nullptr);
		};

		struct RenderTexture
		{
			bgfx::FrameBufferHandle handle;
			bgfx::ViewId vid = 1;
			Texture* realTexture;

			bgfx::TextureFormat::Enum format;

			RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 clearFlags);
		};

		struct Camera
		{
			glm::vec3 position, rotation;
			float FOV = 90.0f;

			float* GetView();
			float* GetProjection();

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

		Mesh* createMesh(Vertex* data, u16* indices, u32 vertSize, u32 triSize, bgfx::VertexLayout pcvDecl);

		struct DrawCall
		{
			u64 state;

			MaterialOverride** overrides;
			size_t overrideCt = 0;

			Mesh* mesh;
			float transformMatrix[4][4];
			Shader* program;
			MaterialState::MatrixMode matrixMode;
		};

		void pushDrawCall(DrawCall d);

		RendererInfo* init();
		void update();
		void shutdown();
	} // namespace engine

	namespace prim
	{
		enum PrimitiveType
		{
			Quad,
			Cube,
			Sphere
		};

		render::Mesh* GetPrimitive(PrimitiveType type);
	}

	namespace engine
	{
		struct EngineInfo
		{
			render::RendererInfo* renderer;
		};

		EngineInfo* init();
		void update();
	}

	namespace math
	{
		float* vec4toArray(glm::vec4 v);
		float* mat4ToArray(glm::mat4 m);
		float** mat3ToArray(glm::mat3 m);

		inline bx::Vec3 convertVec3(glm::vec3 v)
		{
			return bx::Vec3{v.x, v.y, v.z};
		}

		inline glm::vec3 convertVec3(aiVector3D v)
		{
			return glm::vec3{v.x, v.y, v.z};
		}

		glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float t);
		glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t);

		float magnitude(glm::vec3 v);
	}

	namespace time
	{
		float getTime();
		float getSinTime();
		float getCosTime();
		float getDeltaTime();
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
			virtual ~Object();
			glm::vec3 position = glm::vec3(0.0), rotation = glm::vec3(0.0), scale = glm::vec3(1.0);
			string name = GetDefaultName();

			virtual void Update();

			Object* GetParent()
			{
				return parent;
			}

			void SetParent(Object* object);

			std::vector<Object*> GetChildren()
			{
				return children;
			}

			glm::vec3 GetGlobalPosition();
			glm::vec3 GetGlobalRotation();
			glm::vec3 GetGlobalScale();
			glm::mat4 GetTransform();

			void LookAt(glm::vec3 p);

			void SetForward(glm::vec3 f);
			void SetRight(glm::vec3 r);
			void SetUp(glm::vec3 u);

			glm::vec3 GetForward();
			glm::vec3 GetRight();
			glm::vec3 GetUp();


			Object* parent = nullptr;
			std::vector<Object*> children;
			bool active = true;

			Object();

			template <typename ObjectType>
			ObjectType* GetObjectFromType();
		};

		template <typename ObjectType>
		ObjectType* Object::GetObjectFromType()
		{
			if (parent)
			{
				var o = dynamic_cast<ObjectType*>(parent);
				if (o)
					return o;

				for (auto child : parent->children)
				{
					var o = dynamic_cast<ObjectType*>(child);
					if (o)
						return o;
				}
			}

			for (auto child : children)
			{
				var o = dynamic_cast<ObjectType*>(child);
				if (o)
					return o;
			}


			return nullptr;
		}

		struct MeshObject : Object
		{
			render::Material* material;
			render::Mesh* mesh;

			void Update() override;

			string GetDefaultName() override
			{
				return "MeshObject";
			}

			static MeshObject* FromPrimitive(prim::PrimitiveType pt);
		};

		struct CameraObject : Object
		{
			render::Camera* camera;

			CameraObject();

			void Update() override;


			string GetDefaultName() override
			{
				return "Camera";
			}

			static CameraObject* GetMainCamera();
		};

		struct Scene
		{
			Scene();

			std::vector<Object*> objects;
			physics::PhysicalWorld* physicsWorld;

			void Update();

			static Scene* GetMainScene();
		};


		struct ObjectLoader
		{
			struct SceneInfo
			{
				std::vector<render::Mesh*> meshes;
				std::vector<render::Material*> materials;

				const aiScene* scene;

				Object* root;
			};

			static SceneInfo Load(const aiScene* scene);
			static SceneInfo Load(string path);
		};

		void init();
		void update();
	}

	namespace ui
	{
		struct Rect
		{
			float x, y;
			float width, height;

			bool isPointInRect(glm::vec2 p);
		};

		struct SpriteObject : obj::Object
		{
			render::Texture* mainTexture;
			render::Material* material;
			render::Color mainColor;

			void Update() override;

			string GetDefaultName() override
			{
				return "Sprite";
			}
		};

		struct ButtonObject : obj::Object
		{
			void Update() override;

			string GetDefaultName() override
			{
				return "Button";
			}

			int AddHoverEvent(std::function<void()> f);
			int AddClickEvent(std::function<void()> f);

		private:
			std::vector<std::function<void()>> hovers, clicks;

			bool hoverLast, clickLast;
		};
	}

	namespace input
	{
		struct Mouse
		{
			static glm::vec2 GetMousePosition();
			static glm::vec2 GetMouseDelta();

			enum MouseButtonState
			{
				Release = GLFW_RELEASE,
				Press = GLFW_PRESS,
				Hold,
			};

			static MouseButtonState GetMouseButton(int i);
		};

		struct Keyboard
		{
			enum KeyState
			{
				Release = GLFW_RELEASE,
				Press = GLFW_PRESS,
				Hold,
			};

			static KeyState GetKey(int key);
		};
	}

	namespace fs
	{
		template <typename _T>
		using ResDict = std::map<string, _T>;

		struct StringBinaryReader : std::stringstream
		{
			StringBinaryReader(string data);

			READ_FUNC(u16, UInt16);
			READ_FUNC(u8, Byte);

			template <typename T>
			T Read()
			{
				T header;
				read(reinterpret_cast<char*>(&header), sizeof(T));

				return header;
			}

			template <typename T>
			std::vector<T> ReadArray(int count)
			{
				std::vector<T> t;

				for (int i = 0; i < count; i++)
				{
					t.push_back(Read<T>());
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

		struct BinaryReader : std::istream
		{
			enum ByteOrder
			{
				BigEndian,
				LittleEndian
			} byteOrder;

			BinaryReader(std::streambuf* path);

			template <typename T>
			T Read()
			{
				T header;
				read(reinterpret_cast<char*>(&header), sizeof(T));

				return header;
			}

			template <typename T>
			std::vector<T> ReadArray(int count)
			{
				std::vector<T> t;

				for (int i = 0; i < count; i++)
				{
					t.push_back(Read<T>());
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

			u64 ReadUInt64();
			u32 ReadUInt32();
			u16 ReadUInt16();

			s16 ReadInt16();
			s32 ReadInt32();
			s64 ReadInt64();

			ByteOrder ReadByteOrder();
			u8 ReadByte();

			float ReadSingle()
			{
				return Read<float>();
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
	}

	namespace physics
	{
		struct PhysicsBody;

		struct CollisionCallback : btCollisionWorld::ContactResultCallback
		{
			PhysicsBody* body;
			ColliderObject* collider;

			// Overriding the callback method
			btScalar addSingleResult(
				btManifoldPoint& cp,
				const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
				const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1
			) override;
		};

		struct PhysicalWorld
		{
			PhysicalWorld();

			void Update();

			void RemoveBody(int pid, int cpid);

			std::vector<PhysicsBody*> GetGameObjectsCollidingWith(PhysicsBody* collider);
		};

		enum CollisionShape
		{
			Box,
			Sphere,
			Capsule,
			Mesh
		};

		struct ColliderInitInfo
		{
			CollisionShape s;
			glm::vec3 bounds;
			float radius;
			float height;
			render::Mesh* mesh;

			static ColliderInitInfo ForBox(glm::vec3 bounds);
			static ColliderInitInfo ForSphere(float radius);
			static ColliderInitInfo ForCapsule(float radius, float height);
			static ColliderInitInfo ForMesh(render::Mesh* mesh);
		};

		struct CollisionBase
		{
			glm::vec3 contactPoint;
			glm::vec3 normal;
			int faceId;
		};

		struct Collision : CollisionBase
		{
			PhysicsBody* other;
		};

		struct ParticleCollision : CollisionBase
		{
			particle::Particle* other;
		};

		struct ColliderObject : obj::Object
		{
			ColliderInitInfo initInfo;
			ColliderObject(ColliderInitInfo info, Object* parent = nullptr);

		private:
			friend struct PhysicsBody;
			friend CollisionCallback;

			u16 pId = -1;
		};

		struct PhysicsBody : obj::Object
		{
			float mass = 1;

			PhysicsBody(ColliderObject* collisionObj, float mass = 1);


			enum TransformRelationship
			{
				Self, Parent
			} transRelation = Self;

			void Update() override;

			void SetVelocity(glm::vec3 v);
			glm::vec3 GetVelocity();

			void SetAngular(glm::vec3 v);

			void AddImpulse(glm::vec3 v);
			void AddForce(glm::vec3 v);

			void SetLinearFactor(glm::vec3 v);
			void SetAngularFactor(glm::vec3 v);

			void SetDamping(float linear, float angular);

			void AddCollisionEvent(std::function<void(Collision)> func);
			void AddParticleCollisionEvent(std::function<void(ParticleCollision)> func);

			glm::vec3 GetBasisColumn(float v);
			glm::vec3 GetBasisRow(float v);

		private:
			friend PhysicalWorld;
			friend CollisionCallback;

			CollisionCallback* callback_;

			u16 pId = -1;
			u16 cPID = -1;

			std::vector<std::function<void(Collision)>> collisionEvents;
			std::vector<std::function<void(ParticleCollision)>> particleCollisionEvents;

			void OnCollision(Collision c);
			void OnParticleCollision(ParticleCollision c);
		};

		struct Ray
		{
			glm::vec3 position, direction;
			float maxDistance = 10000;

			RaycastHit* Cast();
		};

		struct RaycastHit
		{
			glm::vec3 point, normal;
			PhysicsBody* hit;
		};
	}

	namespace particle
	{
		struct ParticleEmitter;

		struct ParticleSystem
		{
			float duration = 5;
			bool looping = true;

			float startSpeed = 1;
			float startSize = 0.1;
			float startLifetime = 5;

			render::Color startColor;

			struct SystemEmission
			{
				float rateOverTime = 10;
			} emission;

			struct SystemShape
			{
				enum Shape
				{
					Cone
				} type;

				float radius = 1;
				float angle = 35;
			} shape;

			struct SystemCollision
			{
				bool useColliders;
				physics::CollisionShape shape;
				float mass = 1;

				float lifetimeLoss;
			} collision;

			struct SystemRenderer
			{
				enum Mode
				{
					Mesh
				} mode;

				render::Mesh* mesh = GetPrimitive(prim::Sphere);

				render::Material* material;
			} renderer;

			float maxParticles = 100;

			bool playOnStart = true;
		};

		struct Particle
		{
			glm::vec3 position, rotation, scale;
			glm::vec3 velocity;
			render::Color color;
			float lifetime = 0;

			glm::mat4 getTransform();

			glm::vec3 GetUp();
			glm::vec3 GetRight();
			glm::vec3 GetForward();

			ParticleEmitter* emitterParent;

			void OnCollision(physics::Collision c);
			void OnParticleCollision(physics::ParticleCollision c);

			int pId = -1;
			u16 cPID = -1;
		};

		struct ParticleEmitter : obj::Object
		{
			ParticleSystem* system;

			ParticleEmitter();

			void Emit(int amount = 1);
			void Update() override;

		private:
			friend Particle;

			std::vector<Particle*> particles;
			float time = 0;
			bool isPlaying = false;
			float time_alloc = 0;

			std::vector<std::function<void(physics::Collision)>> collisionEvents;

			void OnCollision(physics::Collision c, Particle* p);
		};
	}

	namespace debug
	{
		struct Gizmos
		{
			enum DebugCallType
			{
				Line,
				Sphere
			};

			struct DebugCall
			{
				DebugCallType type;
				glm::vec3 origin, direction;
				float radius;
				render::Color color;
			};

			inline static render::Color color = render::Color::White;

			static void DrawLine(glm::vec3 start, glm::vec3 end);
			static void DrawSphere(glm::vec3 position, float radius);
		};
	}
}

#endif // TOMATO_HPP
