#include "tomato.hpp"
#include "bx/math.h"
#include "bimg/include/bimg/bimg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define PAR_SHAPES_IMPLEMENTATION
#include <complex.h>

#include "par_shapes.h"

#include "tomato_generated/cube.h"

static void glfw_errorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}


tmt::render::Color tmt::render::Color::White = {1, 1, 1, 1};
tmt::render::Color tmt::render::Color::Blue = {0, 0, 1, 1};
tmt::render::Color tmt::render::Color::Green = {0, 1, 0, 1};
tmt::render::Color tmt::render::Color::Red = {1, 0, 0, 1};

static tmt::render::RendererInfo* renderer;
std::vector<tmt::render::DrawCall> calls;
std::vector<tmt::debug::Gizmos::DebugCall> debugCalls;


glm::vec2 mousep = {0, 0};
glm::vec2 mousedelta = {0, 0};

tmt::render::Camera* mainCamera;
tmt::obj::Scene* mainScene;

tmt::render::Shader* defaultShader;

int counterTime = 0;
float deltaTime = 1.0f / 60.0f;
float lastTime = 0;

bgfx::UniformHandle orthoHandle, timeHandle, vposHandle;

bool subHandlesLoaded = false;

std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*> primitives = std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*>();

bgfx::VertexLayout tmt::render::Vertex::getVertexLayout()
{
	var layout = bgfx::VertexLayout();
	layout.begin()
	      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	      .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
	      .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
	      .end();

	return layout;
}

tmt::render::SubShader::SubShader(string name, ShaderType type)
{
	string shaderPath = "";

	switch (bgfx::getRendererType())
	{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: shaderPath = "runtime/shaders/dx/";
			break;
		case bgfx::RendererType::Gnm: shaderPath = "shaders/pssl/";
			break;
		case bgfx::RendererType::Metal: shaderPath = "shaders/metal/";
			break;
		case bgfx::RendererType::OpenGL: shaderPath = "runtime/shaders/gl/";
			break;
		case bgfx::RendererType::OpenGLES: shaderPath = "shaders/essl/";
			break;
		case bgfx::RendererType::Vulkan: shaderPath = "runtime/shaders/spirv/";
			break;
		//case bgfx::RendererType::Nvn:
		//case bgfx::RendererType::WebGPU:
		case bgfx::RendererType::Count: handle = BGFX_INVALID_HANDLE;
			return; // count included to keep compiler warnings happy
	}

	shaderPath += name;

	switch (type)
	{
		case Vertex: shaderPath += ".cvbsh";
			break;
		case Fragment: shaderPath += ".cfbsh";
			break;
		case Compute: shaderPath += ".ccbsh";
			break;
		//default: shaderPath += ".cbsh"; break;
	}

	std::ifstream in(shaderPath, std::ifstream::ate | std::ifstream::binary);

	in.seekg(0, std::ios::end);
	std::streamsize size = in.tellg();
	in.seekg(0, std::ios::beg);

	const bgfx::Memory* mem = bgfx::alloc(size);
	in.read(reinterpret_cast<char*>(mem->data), size);

	in.close();

	handle = createShader(mem);

	var unis = std::vector<bgfx::UniformHandle>();
	var uniformCount = getShaderUniforms(handle);
	unis.resize(uniformCount);
	getShaderUniforms(handle, unis.data(), uniformCount);

	std::vector<string> uniNames;

	for (int i = 0; i < uniformCount; i++)
	{
		bgfx::UniformInfo info = {};

		getUniformInfo(unis[i], info);

		if (std::find(uniNames.begin(), uniNames.end(), info.name) == uniNames.end())
		{
			string iname = info.name;

			if (iname.rfind("iu_", 0) == 0)
			{
				// pos=0 limits the search to the prefix
				continue;
			}

			var uniform = new ShaderUniform();

			uniform->name = info.name;
			uniform->type = info.type;
			uniform->handle = unis[i];


			uniforms.push_back(uniform);
			uniNames.push_back(info.name);
		}
	}
}

tmt::render::ShaderUniform* tmt::render::SubShader::GetUniform(string name)
{
	for (var uni : uniforms)
	{
		if (uni->name == name)
			return uni;
	}

	return {};
}

tmt::render::Shader::Shader(ShaderInitInfo info)
{
	program = createProgram(info.vertexProgram->handle, info.fragmentProgram->handle, true);

	subShaders.push_back(info.vertexProgram);
	subShaders.push_back(info.fragmentProgram);
}

void tmt::render::Shader::Push(int viewId, MaterialOverride** overrides, size_t oc)
{
	std::map<std::string, MaterialOverride> m_overrides;

	if (overrides != nullptr)
	{
		for (int i = 0; i < oc; ++i)
		{
			var name = overrides[i]->name;

			var pair = std::make_pair<std::string, MaterialOverride>(name, *overrides[i]);
			m_overrides.insert(pair);
		}
	}

	for (var shader : subShaders)
	{
		for (var uni : shader->uniforms)
		{
			if (overrides != nullptr)
			{
				if (m_overrides.contains(uni->name))
				{
					var ovr = m_overrides.find(uni->name)->second;
					uni->v4 = ovr.v4;
					uni->m3 = ovr.m3;
					uni->m4 = ovr.m4;
					uni->tex = ovr.tex;
				}
			}

			uni->Use();
		}
	}


	submit(viewId, program);
}

tmt::render::ComputeShader::ComputeShader(SubShader* shader)
{
	program = createProgram(shader->handle, true);
	internalShader = shader;
}

void tmt::render::ComputeShader::SetUniform(string name, bgfx::UniformType::Enum type, const void* data)
{
	var uni = createUniform(name.c_str(), type);

	setUniform(uni, data);

	destroy(uni);
}

void tmt::render::ComputeShader::SetMat4(string name, glm::mat4 m)
{
	//internalShader->GetUniform()
	for (int i = 0; i < 4; ++i)
	{
		SetUniform(name + "[" + std::to_string(i) + "]", bgfx::UniformType::Vec4, math::vec4toArray(m[0]));
	}
}

void tmt::render::ComputeShader::Run(int vid, glm::vec3 groups)
{
	for (var uni : internalShader->uniforms)
	{
		uni->Use();
	}

	dispatch(vid, program, groups.x, groups.y, groups.z);
}


tmt::render::MaterialOverride* tmt::render::Material::GetUniform(string name)
{
	for (auto override : overrides)
		if (override->name == name) return override;

	return nullptr;
}

u64 tmt::render::Material::GetMaterialState()
{
	u64 v = state.cull;
	v |= state.depth;
	v |= BGFX_STATE_WRITE_MASK;

	return v;
}

tmt::render::Material::Material(Shader* shader)
{
	Reload(shader);
}

void tmt::render::Material::Reload(Shader* shader)
{
	if (shader)
	{
		this->shader = shader;
		for (auto sub_shader : shader->subShaders)
		{
			for (auto uniform : sub_shader->uniforms)
			{
				var ovr = new MaterialOverride();
				ovr->name = uniform->name;

				overrides.push_back(ovr);
			}
		}
	}
}

void tmt::render::Mesh::draw(glm::mat4 transform, Material* material)
{
	var drawCall = DrawCall();

	drawCall.mesh = this;
	drawCall.state = material->GetMaterialState();
	drawCall.matrixMode = material->state.matrixMode;

	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			drawCall.transformMatrix[x][y] = transform[x][y];
		}
	}

	drawCall.program = material->shader;
	if (material->overrides.size() > 0)
	{
		drawCall.overrides = material->overrides.data();
		drawCall.overrideCt = material->overrides.size();
	}
	else
		drawCall.overrides = nullptr;

	pushDrawCall(drawCall);
}

tmt::render::Model::Model(string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.
		ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		var msh = scene->mMeshes[i];

		std::vector<Vertex> vertices;

		for (int j = 0; j < msh->mNumVertices; ++j)
		{
			var pos = msh->mVertices[j];
			var norm = msh->mNormals[j];
			var uv = msh->mTextureCoords[0][j];

			var vertex = Vertex{};

			vertex.position = glm::vec3{pos.x, pos.y, pos.z};
			vertex.normal = glm::vec3{norm.x, norm.y, norm.z};
			vertex.uv0 = glm::vec2{uv.x, uv.y};
			vertices.push_back(vertex);
		}

		std::vector<u16> indices;

		for (unsigned int i = 0; i < msh->mNumFaces; i++)
		{
			aiFace face = msh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		meshes.push_back(createMesh(vertices.data(), indices.data(), vertices.size(), indices.size(),
		                            Vertex::getVertexLayout()));
		materialIndices.push_back(msh->mMaterialIndex);
	}
}

tmt::render::Model::Model(const aiScene* scene)
{
	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		var msh = scene->mMeshes[i];

		std::vector<Vertex> vertices;

		for (int j = 0; j < msh->mNumVertices; ++j)
		{
			var pos = msh->mVertices[j];
			var norm = msh->mNormals[j];
			var uv = msh->mTextureCoords[0][j];

			var vertex = Vertex{};

			vertex.position = glm::vec3{pos.x, pos.y, pos.z};
			vertex.normal = glm::vec3{norm.x, norm.y, norm.z};
			vertex.uv0 = glm::vec2{uv.x, uv.y};
			vertices.push_back(vertex);
		}

		std::vector<u16> indices;

		for (unsigned int i = 0; i < msh->mNumFaces; i++)
		{
			aiFace face = msh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		meshes.push_back(createMesh(vertices.data(), indices.data(), vertices.size(), indices.size(),
		                            Vertex::getVertexLayout()));
		materialIndices.push_back(msh->mMaterialIndex);
	}
}

tmt::render::Mesh* tmt::render::createMesh(Vertex* data, u16* indices, u32 vertCount, u32 triSize,
                                           bgfx::VertexLayout layout)
{
	u16 stride = layout.getStride();
	u16 vertS = stride * vertCount;
	u16 indeS = sizeof(u16) * triSize;

	var mesh = new Mesh();
	mesh->vertices = data;
	mesh->indices = indices;

	{
		const bgfx::Memory* mem = bgfx::alloc(vertS);

		bx::memCopy(mem->data, data, vertS);

		bgfx::VertexBufferHandle vbh = createVertexBuffer(mem, layout);
		mesh->vbh = vbh;
		mesh->vertexCount = vertCount;

		auto positions = new glm::vec3[vertCount];
		auto normals = new glm::vec3[vertCount];
		auto uvs = new glm::vec2[vertCount];

		for (int i = 0; i < vertCount; ++i)
		{
			var vertex = data[i];
			positions[i] = vertex.position;
			normals[i] = vertex.normal;
			uvs[i] = vertex.uv0;
		}

		bgfx::VertexLayout playout;
		playout.begin();
		playout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		playout.end();

		bgfx::VertexLayout nlayout;
		nlayout.begin();
		nlayout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
		nlayout.end();
		bgfx::VertexLayout ulayout;
		ulayout.begin();
		ulayout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
		ulayout.end();


		u16 flags = BGFX_BUFFER_COMPUTE_READ;
		{
			var vsz = sizeof(glm::vec3) * vertCount;

			const bgfx::Memory* vmem = bgfx::alloc(vsz);

			bx::memCopy(vmem->data, positions, vsz);

			bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(
				vmem, playout, flags
			);

			mesh->vertexBuffers.push_back(vertexBuffer);
		}

		{
			var vsz = sizeof(glm::vec3) * vertCount;

			const bgfx::Memory* vmem = bgfx::alloc(vsz);

			bx::memCopy(vmem->data, normals, vsz);

			bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(
				vmem, nlayout, flags
			);

			mesh->vertexBuffers.push_back(vertexBuffer);
		}
		{
			var vsz = sizeof(glm::vec2) * vertCount;

			const bgfx::Memory* vmem = bgfx::alloc(vsz);

			bx::memCopy(vmem->data, uvs, vsz);

			bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(
				vmem, ulayout, flags
			);

			mesh->vertexBuffers.push_back(vertexBuffer);
		}
	}

	{
		const bgfx::Memory* mem = bgfx::alloc(indeS);

		bx::memCopy(mem->data, indices, indeS);

		bgfx::IndexBufferHandle ibh = createIndexBuffer(mem, BGFX_BUFFER_COMPUTE_READ);

		mesh->ibh = ibh;
		mesh->indexCount = triSize;

		const bgfx::Memory* vmem = bgfx::alloc(indeS);

		bx::memCopy(vmem->data, indices, indeS);

		mesh->indexBuffer = createDynamicIndexBuffer(
			vmem, BGFX_BUFFER_COMPUTE_READ
		);
	}

	return mesh;
}

void tmt::render::pushDrawCall(DrawCall d)
{
	calls.push_back(d);
}

tmt::render::RendererInfo* tmt::render::init()
{
	glfwSetErrorCallback(glfw_errorCallback);
	if (!glfwInit())
		return nullptr;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "helloworld", nullptr, nullptr);
	if (!window)
		return nullptr;
	//glfwSetKeyCallback(window, glfw_keyCallback);
	// Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
	// Most graphics APIs must be used on the same thread that created the window.
	bgfx::renderFrame();

	bgfx::Init init;

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD

#elif BX_PLATFORM_OSX

#elif BX_PLATFORM_WINDOWS
	init.platformData.nwh = glfwGetWin32Window(window);
#endif

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	init.resolution.width = static_cast<uint32_t>(width);
	init.resolution.height = static_cast<uint32_t>(height);
	init.resolution.reset = BGFX_RESET_VSYNC;
	//init.debug = true;


	init.vendorId = BGFX_PCI_ID_NVIDIA;

	//init.type = bgfx::RendererType::OpenGL;

	if (!bgfx::init(init))
		return nullptr;
	// Set view 0 to the same dimensions as the window and to clear the color buffer.
	constexpr bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
	setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

	var m_debug = BGFX_DEBUG_TEXT;

	bgfx::setDebug(m_debug);

	var caps = bgfx::getCaps();

	if ((caps->supported & BGFX_CAPS_TEXTURE_BLIT) == 0)
	{
		exit(-69);
	}

	renderer = new RendererInfo();
	calls = std::vector<DrawCall>();

	renderer->clearView = kClearView;
	renderer->window = window;

	renderer->windowWidth = width;
	renderer->windowHeight = height;

	ShaderInitInfo info = {
		new SubShader("test/vert", SubShader::Vertex),
		new SubShader("test/frag", SubShader::Fragment),
	};

	defaultShader = new Shader(info);

	bgfx::loadRenderDoc();

	return renderer;
}

void tmt::render::update()
{
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(renderer->windowWidth),
	                  static_cast<uint16_t>(renderer->windowHeight));

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::dbgTextClear();

	const bgfx::Stats* stats = bgfx::getStats();

	bgfx::dbgTextPrintf(5, 5, 0x0f, "%s", getRendererName(bgfx::getRendererType()));

	bgfx::setDebug(BGFX_DEBUG_TEXT);

	float proj[16];
	float ortho[16];
	float oneMat[16];

	for (int i = 0; i < 16; ++i)
	{
		proj[i] = 1;
		oneMat[i] = 1;
		ortho[i] = 1;
	}

	if (mainCamera)
	{
		bx::mtxProj(proj, mainCamera->FOV,
		            static_cast<float>(renderer->windowWidth) / static_cast<float>(renderer->windowHeight), 0.01f,
		            100.0f, bgfx::getCaps()->homogeneousDepth);
		bx::mtxOrtho(ortho, 0, static_cast<float>(renderer->windowWidth), static_cast<float>(renderer->windowHeight),
		             0.0f, -1, 100.0f, 0, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, mainCamera->GetView(), proj);
	}

	if (!subHandlesLoaded)
	{
		orthoHandle = createUniform("iu_ortho", bgfx::UniformType::Mat4);
		timeHandle = createUniform("iu_time", bgfx::UniformType::Vec4);
		vposHandle = createUniform("iu_viewPos", bgfx::UniformType::Vec4);

		subHandlesLoaded = true;
	}

	float t[4] = {
		static_cast<float>(counterTime), static_cast<float>(glm::sin(counterTime)),
		static_cast<float>(glm::cos(counterTime)), 0
	};
	for (auto call : calls)
	{
		bgfx::setTransform(call.transformMatrix);

		if (mainCamera)
		{
			switch (call.matrixMode)
			{
				case MaterialState::ViewProj:
					bgfx::setViewTransform(0, mainCamera->GetView(), proj);
					break;
				case MaterialState::View:
					//bgfx::setViewTransform(0, mainCamera->GetView(), oneMat);
					break;
				case MaterialState::Proj:
					//bgfx::setViewTransform(0, oneMat, proj);
					break;
				case MaterialState::None:
					//bgfx::setViewTransform(0, oneMat, oneMat);
					break;
				case MaterialState::ViewOrthoProj:
					//bgfx::setViewTransform(0, mainCamera->GetView(), ortho);
					setUniform(orthoHandle, ortho);
					break;
				case MaterialState::OrthoProj:
					//bgfx::setViewTransform(0, oneMat, ortho);
					setUniform(orthoHandle, ortho);
					break;
			}
		}

		setUniform(timeHandle, t);
		if (mainCamera)
		{
			setUniform(vposHandle, math::vec4toArray(glm::vec4(mainCamera->position, 0)));
		}

		setVertexBuffer(0, call.mesh->vbh);
		setIndexBuffer(call.mesh->ibh);

		bgfx::setState(call.state);

		call.program->Push(0, call.overrides, call.overrideCt);

		//bgfx::discard();
	}
	calls.clear();

	bgfx::setViewTransform(0, mainCamera->GetView(), proj);

	auto dde = DebugDrawEncoder();

	dde.begin(0);

	for (auto d : debugCalls)
	{
		dde.setWireframe(true);
		dde.setColor(d.color.getHex());
		switch (d.type)
		{
			case debug::Gizmos::Line:
				{
					dde.push();

					dde.moveTo(math::convertVec3(d.origin));
					dde.lineTo(math::convertVec3(d.direction));

					dde.pop();
				}
				break;
			case debug::Gizmos::Sphere:
				{
					dde.push();

					//dde.moveTo(math::convertVec3(d.origin));
					dde.setWireframe(false);
					dde.drawOrb(d.origin.x, d.origin.y, d.origin.z, d.radius);

					dde.pop();
				}
				break;
		}
	}


	dde.end();

	debugCalls.clear();

	bgfx::frame();
	glfwPollEvents();
	counterTime++;

	bgfx::touch(0);
	//bgfx::touch(1);
}

void tmt::render::shutdown()
{
	bgfx::shutdown();
	glfwTerminate();
}

tmt::engine::EngineInfo* tmt::engine::init()
{
	var rendererInfo = render::init();

	var engineInfo = new EngineInfo();
	engineInfo->renderer = rendererInfo;

	obj::init();

	return engineInfo;
}

void tmt::engine::update()
{
	obj::update();
	render::update();

	double xpos = 0;
	double ypos = 0;

	glfwGetCursorPos(renderer->window, &xpos, &ypos);

	var p = glm::vec2{xpos, ypos};

	mousedelta = mousep - p;
	mousep = p;

	deltaTime = glfwGetTime() - lastTime;
	lastTime = glfwGetTime();
}

tmt::render::Texture::Texture(string path)
{
	int nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);


	uint64_t textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_POINT; // Adjust as needed
	bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA8;

	// Create the texture in bgfx, passing the image data directly
	handle = createTexture2D(
		static_cast<uint16_t>(width),
		static_cast<uint16_t>(height),
		false, // no mip-maps
		1, // single layer
		textureFormat,
		textureFlags,
		bgfx::copy(data, width * height * nrChannels) // copies the image data
	);

	stbi_image_free(data);

	format = textureFormat;
}

tmt::render::Texture::Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory* mem)
{
	handle = createTexture2D(
		width, height,
		false, 1,
		tf,
		flags,
		mem
	);

	format = tf;
	this->width = width;
	this->height = height;
}

tmt::render::RenderTexture::RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 cf)
{
	const bgfx::Memory* mem = nullptr;
	if (format == bgfx::TextureFormat::RGBA8)
	{
		//std::vector<GLubyte> pixels(width * height * 4, (GLubyte)0xffffffff);
		//mem = bgfx::copy(pixels.data(), width * height* 4);
	}

	realTexture = new Texture(width, height, format,
	                          BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT |
	                          BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, mem);

	handle = createFrameBuffer(1, &realTexture->handle, false);
	this->format = format;

	bgfx::setViewName(vid, "RenderTexture");
	bgfx::setViewClear(vid, cf);
	bgfx::setViewRect(vid, 0, 0, width, height);
	setViewFrameBuffer(vid, handle);
}

float* tmt::render::Camera::GetView()
{
	var Front = GetFront();
	var Up = GetUp();

	float view[16];

	mtxLookAt(view, bx::Vec3(position.x, position.y, position.z), math::convertVec3(position + Front),
	          bx::Vec3(Up.x, Up.y, Up.z));

	return view;
}

float* tmt::render::Camera::GetProjection()
{
	float proj[16];

	bx::mtxProj(proj, (FOV), static_cast<float>(renderer->windowWidth) / static_cast<float>(renderer->windowHeight),
	            0.01f, 100.0f, bgfx::getCaps()->homogeneousDepth);
	return proj;
}

glm::vec3 tmt::render::Camera::GetFront()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	direction.y = sin(glm::radians(rotation.x));
	direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	glm::vec3 Front = normalize(direction);

	return Front;
}

glm::vec3 tmt::render::Camera::GetUp()
{
	var Front = GetFront();

	glm::vec3 Right = normalize(cross(Front, glm::vec3{0, 1, 0}));
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	glm::vec3 Up = normalize(cross(Right, Front));

	return Up;
}

tmt::render::Camera* tmt::render::Camera::GetMainCamera()
{
	return mainCamera;
}

tmt::render::Camera::Camera()
{
	mainCamera = this;
}

std::tuple<tmt::render::Vertex*, u16*, u16, u16> convertMesh(par_shapes_mesh* mesh)
{
	par_shapes_compute_normals(mesh);

	// Number of vertices and indices
	var vertexCount = mesh->npoints;
	var indexCount = mesh->ntriangles * 3;

	// Allocate memory for vertices and indices
	var vertices = new tmt::render::Vertex[vertexCount];
	var indices = new uint16_t[indexCount];

	// Populate vertices
	for (int i = 0; i < vertexCount; ++i)
	{
		tmt::render::Vertex& vertex = vertices[i];

		// Extract position
		vertex.position = glm::vec3(
			mesh->points[3 * i + 0],
			mesh->points[3 * i + 1],
			mesh->points[3 * i + 2]
		);

		// Extract normal if available
		if (mesh->normals)
		{
			vertex.normal = glm::vec3(
				mesh->normals[3 * i + 0],
				mesh->normals[3 * i + 1],
				mesh->normals[3 * i + 2]
			);
		}

		// Extract UV coordinates if available
		if (mesh->tcoords)
		{
			vertex.uv0 = glm::vec2(
				mesh->tcoords[2 * i + 0],
				mesh->tcoords[2 * i + 1]
			);
		}
	}

	// Populate indices
	for (int i = 0; i < mesh->ntriangles; ++i)
	{
		indices[3 * i + 0] = mesh->triangles[3 * i + 0];
		indices[3 * i + 1] = mesh->triangles[3 * i + 1];
		indices[3 * i + 2] = mesh->triangles[3 * i + 2];
	}
	return {vertices, indices, vertexCount, indexCount};
}


tmt::render::Mesh* tmt::prim::GetPrimitive(PrimitiveType type)
{
	if (!primitives.contains(type))
	{
		Vertex* vertices;
		u16* indices;

		size_t vertCount, indCount;

		switch (type)
		{
			case Quad:
				{
					vertices = new Vertex[4]
					{
						{glm::vec3{0, 0, 0}, glm::vec3{1}, glm::vec2{0, 0}},
						{glm::vec3{1, 0, 0}, glm::vec3{1}, glm::vec2{1, 0}},
						{glm::vec3{1, 1, 0}, glm::vec3{1}, glm::vec2{1, 1}},
						{glm::vec3{0, 1, 0}, glm::vec3{1}, glm::vec2{0, 1}},
					};

					indices = new u16[6]
					{
						2, 3, 0,
						0, 1, 2,
					};

					vertCount = 4;
					indCount = 6;
				}

				break;
			case Cube:
				{
					vertCount = cube_mesh::vertexCount_0;
					indCount = cube_mesh::indexCount_0;
					vertices = cube_mesh::vertices_0;
					indices = cube_mesh::indices_0;
				}
				break;
			case Sphere:
				{
					var mesh = par_shapes_create_parametric_sphere(12, 12);
					var [verts, inds, vc, ic] = convertMesh(mesh);
					vertCount = vc;
					indCount = ic;
					vertices = verts;
					indices = inds;
					par_shapes_free_mesh(mesh);
				}
				break;
		}

		var mesh = createMesh(vertices, indices, vertCount, indCount, Vertex::getVertexLayout());

		primitives.insert(std::make_pair(type, mesh));
	}


	return primitives[type];
}

void tmt::render::ShaderUniform::Use()
{
	switch (type)
	{
		case bgfx::UniformType::Sampler:
			if (tex)
			{
				setTexture(0, handle, tex->handle);
			}
			break;
		case bgfx::UniformType::End:
			break;
		case bgfx::UniformType::Vec4:
			setUniform(handle, math::vec4toArray(v4));
			break;
		case bgfx::UniformType::Mat3:
			setUniform(handle, math::mat3ToArray(m3));
			break;
		case bgfx::UniformType::Mat4:
			{
				float m[4][4];
				for (int x = 0; x < 4; ++x)
				{
					for (int y = 0; y < 4; ++y)
					{
						m[x][y] = m4[x][y];
					}
				}

				setUniform(handle, m);
			}
			break;
		case bgfx::UniformType::Count:
			break;
		default:
			break;
	}
}

float* tmt::math::vec4toArray(glm::vec4 v)
{
	float f[4] = {v.x, v.y, v.z, v.w};
	return f;
}

float* tmt::math::mat4ToArray(glm::mat4 m)
{
	float t[4][4];
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			t[x][y] = m[x][y];
		}
	}

	return reinterpret_cast<float*>(t);
}

float** tmt::math::mat3ToArray(glm::mat3 m)
{
	return nullptr;
}

glm::vec3 tmt::math::slerp(glm::vec3 start, glm::vec3 end, float t)
{
	// Normalize the input vectors
	glm::vec3 startNormalized = normalize(start);
	glm::vec3 endNormalized = normalize(end);

	// Compute the dot product
	float dot = glm::dot(startNormalized, endNormalized);

	// Clamp the dot product to avoid numerical errors
	dot = glm::clamp(dot, -1.0f, 1.0f);

	// Calculate the angle between the vectors
	float theta = acos(dot) * t;

	// Compute the second quaternion using spherical interpolation
	glm::vec3 endPerpendicular = normalize(endNormalized - startNormalized * dot);

	// Perform spherical interpolation and return the result
	return startNormalized * cos(theta) + endPerpendicular * sin(theta);
}

float tmt::math::magnitude(glm::vec3 v)
{
	float m = glm::sqrt(glm::pow(v.x, 2) + glm::pow(v.y, 2) + glm::pow(v.z, 2));

	if (m < 0)
	{
		m = 0;
	}

	return m;
}

tmt::obj::Object::~Object()
{
}

void tmt::obj::Object::Update()
{
	for (auto child : children)
	{
		if (child->active)
			child->Update();
	}
}

void tmt::obj::Object::SetParent(Object* object)
{
	if (parent)
	{
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
		parent = nullptr;
	}

	var scf = std::find(mainScene->objects.begin(), mainScene->objects.end(), this);
	parent = object;
	if (parent)
	{
		parent->children.push_back(this);
		if (scf != mainScene->objects.end())
		{
			mainScene->objects.erase(scf);
		}
	}
	else
	{
		if (scf == mainScene->objects.end())
		{
			mainScene->objects.push_back(this);
		}
	}
}

glm::vec3 tmt::obj::Object::GetGlobalPosition()
{
	var p = position;

	if (parent)
		p += parent->GetGlobalPosition();

	return p;
}

glm::vec3 tmt::obj::Object::GetGlobalRotation()
{
	var p = rotation;

	if (parent)
		p += parent->GetGlobalRotation();

	return p;
}

glm::vec3 tmt::obj::Object::GetGlobalScale()
{
	var p = scale;

	if (parent)
		p *= parent->GetGlobalScale();

	return p;
}

glm::mat4 tmt::obj::Object::GetTransform()
{
	var pos = GetGlobalPosition();
	var rot = GetGlobalRotation();
	var scl = GetGlobalScale();

	var r = rot;
	var rx = rotate(glm::mat4(1.0), glm::radians(r.x), {1, 0, 0});
	var ry = rotate(glm::mat4(1.0), glm::radians(r.y), {0, 1, 0});
	var rz = rotate(glm::mat4(1.0), glm::radians(r.z), {0, 0, 1});

	var rt = ry * rx * rz;

	return translate(glm::mat4(1.0), pos) * rt * glm::scale(glm::mat4(1.0), scl);
}

void tmt::obj::Object::LookAt(glm::vec3 p)
{
	glm::vec3 direction = normalize(p - position);

	float pitch = glm::degrees(asin(direction.y)); // Calculate pitch from Y component
	float yaw = glm::degrees(atan2(direction.z, direction.x)); // Calculate yaw from X and Z

	rotation = glm::vec3(pitch, yaw, rotation.z); // Roll is 0 by default
}

glm::quat LookRotation(glm::vec3 forward, glm::vec3 up)
{
	glm::vec3 f = normalize(forward); // Ensure forward is normalized
	glm::vec3 r = normalize(cross(up, f)); // Right vector
	glm::vec3 u = cross(f, r); // Corrected up vector

	glm::mat3 rotationMatrix(r, u, f); // Right, Up, Forward form the basis vectors
	return quat_cast(rotationMatrix); // Convert to quaternion
}

glm::quat FromToRotation(glm::vec3 from, glm::vec3 to)
{
	glm::vec3 f = normalize(from);
	glm::vec3 t = normalize(to);

	float dotProduct = glm::dot(f, t);
	if (dotProduct >= 1.0f)
	{
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // No rotation needed
	}
	if (dotProduct <= -1.0f)
	{
		// Handle 180-degree rotation
		glm::vec3 axis = normalize(cross(glm::vec3(1, 0, 0), f));
		if (glm::length(axis) < 0.001f)
		{
			axis = normalize(cross(glm::vec3(0, 1, 0), f));
		}
		return angleAxis(glm::pi<float>(), axis);
	}

	glm::vec3 crossProduct = cross(f, t);
	float s = glm::sqrt((1 + dotProduct) * 2);
	float invS = 1 / s;

	return glm::quat(
		s * 0.5f,
		crossProduct.x * invS,
		crossProduct.y * invS,
		crossProduct.z * invS
	);
}

void tmt::obj::Object::SetForward(glm::vec3 f)
{
	glm::quat quaternion = LookRotation(f, {0, 1, 0});
	rotation = degrees(eulerAngles(quaternion));
}

void tmt::obj::Object::SetRight(glm::vec3 r)
{
	glm::quat quaternion = FromToRotation(glm::vec3{1, 0, 0}, r);
	rotation = degrees(eulerAngles(quaternion));
}

void tmt::obj::Object::SetUp(glm::vec3 u)
{
	glm::quat quaternion = FromToRotation(glm::vec3{0, 1, 0}, u);
	rotation = degrees(eulerAngles(quaternion));
}

glm::vec3 tmt::obj::Object::GetForward()
{
	var quat = glm::quat(radians(rotation));

	return quat * glm::vec3{0, 0, 1};
}

glm::vec3 tmt::obj::Object::GetRight()
{
	var quat = glm::quat(radians(rotation));

	return quat * glm::vec3{1, 0, 0};
}

glm::vec3 tmt::obj::Object::GetUp()
{
	var quat = glm::quat(radians(rotation));

	return quat * glm::vec3{0, 1, 0};
}

tmt::obj::Object::Object()
{
	mainScene->objects.push_back(this);
}

void tmt::obj::MeshObject::Update()
{
	if (mesh != nullptr && material != nullptr)
		mesh->draw(GetTransform(), material);

	Object::Update();
}

tmt::obj::MeshObject* tmt::obj::MeshObject::FromPrimitive(prim::PrimitiveType pt)
{
	var mesh = new MeshObject();

	mesh->mesh = GetPrimitive(pt);
	mesh->material = new render::Material(defaultShader);
	mesh->material->GetUniform("u_color")->v4 = render::Color::White.getData();

	return mesh;
}

tmt::obj::CameraObject* mainCameraObject;

tmt::obj::CameraObject::CameraObject() : Object()
{
	camera = new render::Camera();
	mainCameraObject = this;
}

void tmt::obj::CameraObject::Update()
{
	//LookAt(glm::vec3{ 0,0,0 });

	camera->position = GetGlobalPosition();
	camera->rotation = rotation;

	Object::Update();
}


tmt::obj::CameraObject* tmt::obj::CameraObject::GetMainCamera()
{
	return mainCameraObject;
}

tmt::obj::Scene::Scene()
{
	mainScene = this;

	physicsWorld = new physics::PhysicalWorld();
}

void tmt::obj::Scene::Update()
{
	for (auto object : objects)
		if (object->active)
			object->Update();

	physicsWorld->Update();
}

tmt::obj::Scene* tmt::obj::Scene::GetMainScene()
{
	return mainScene;
}

tmt::obj::Object* LoadObject(aiNode* node, tmt::obj::ObjectLoader::SceneInfo info)
{
	var obj = new tmt::obj::Object();
	obj->name = node->mName.C_Str();

	var transform = node->mTransformation;
	aiVector3D p, r, s;

	transform.Decompose(s, r, p);

	r.x = glm::degrees(r.x);
	r.y = glm::degrees(r.y);
	r.z = glm::degrees(r.z);

	obj->position = tmt::math::convertVec3(p);
	obj->rotation = tmt::math::convertVec3(r);
	obj->scale = tmt::math::convertVec3(s);

	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		var meshIndex = node->mMeshes[i];

		var msh = new tmt::obj::MeshObject();
		msh->mesh = info.meshes[meshIndex];
		msh->material = info.materials[info.scene->mMeshes[meshIndex]->mMaterialIndex];

		msh->SetParent(obj);
	}

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		var cobj = LoadObject(node->mChildren[i], info);

		cobj->SetParent(obj);
	}

	return obj;
}

tmt::obj::ObjectLoader::SceneInfo tmt::obj::ObjectLoader::Load(const aiScene* scene)
{
	var sceneInfo = SceneInfo{};

	var mdl = new render::Model(scene);

	sceneInfo.meshes = mdl->meshes;
	for (int i = 0; i < scene->mNumMaterials; ++i)
	{
		sceneInfo.materials.push_back(new render::Material(nullptr));
	}
	sceneInfo.scene = scene;

	sceneInfo.root = LoadObject(scene->mRootNode, sceneInfo);

	return sceneInfo;
}

tmt::obj::ObjectLoader::SceneInfo tmt::obj::ObjectLoader::Load(string path)
{
	if (path.substr(path.find_last_of(".") + 1) == "bfres")
	{
		std::cout << "Yes..." << std::endl;
	}
	else if (path.substr(path.find_last_of(".") + 1) == "tmobj")
	{
		std::cout << "Yes..." << std::endl;
	}
	else
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return {};
		}

		return Load(scene);
	}

	return {};
}

void tmt::obj::init()
{
	var ms = new Scene();
}

void tmt::obj::update()
{
	mainScene->Update();
}

bool tmt::ui::Rect::isPointInRect(glm::vec2 p)
{
	if (p.x >= x && p.x <= x + width)
	{
		if (p.y >= y && p.y <= y + height)
		{
			return true;
		}
	}
	return false;
}

void tmt::ui::SpriteObject::Update()
{
	var tex = material->GetUniform("s_texColor");
	var color = material->GetUniform("u_color");

	color->v4 = mainColor.getData();
	tex->tex = mainTexture;

	material->state.write = BGFX_STATE_WRITE_RGB;
	material->state.depth = render::MaterialState::Always;

	var drawCall = render::DrawCall();

	drawCall.mesh = GetPrimitive(prim::Quad);
	drawCall.state = material->GetMaterialState();
	drawCall.matrixMode = render::MaterialState::OrthoProj;

	var transform = GetTransform();

	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			drawCall.transformMatrix[x][y] = transform[x][y];
		}
	}

	drawCall.program = material->shader;
	if (material->overrides.size() > 0)
	{
		drawCall.overrides = material->overrides.data();
		drawCall.overrideCt = material->overrides.size();
	}
	else
		drawCall.overrides = nullptr;

	pushDrawCall(drawCall);


	Object::Update();
}

void tmt::ui::ButtonObject::Update()
{
	var pos = input::Mouse::GetMousePosition();

	var gpos = GetGlobalPosition();
	var gscl = GetGlobalScale();
	var rect = Rect{gpos.x, gpos.y, gscl.x, gscl.y};

	bool hover = rect.isPointInRect(pos);
	bool click = false;

	// mama a typecheck behind u
	var sprite = dynamic_cast<SpriteObject*>(parent);

	float darkenAmt = 0.25;

	if (hover)
	{
		if (!hoverLast)
		{
			if (sprite)
			{
				sprite->mainColor.darken(-darkenAmt);
			}
		}
		for (const auto& function : hovers)
		{
			function();
		}

		if (input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == input::Mouse::Press)
		{
			click = true;
			if (!clickLast)
			{
				for (const auto& function : clicks)
				{
					function();
				}
			}
		}
	}
	else
	{
		if (hoverLast)
		{
			if (sprite)
			{
				sprite->mainColor.darken(darkenAmt);
			}
		}
	}

	hoverLast = hover;
	clickLast = click;
}

int tmt::ui::ButtonObject::AddHoverEvent(std::function<void()> f)
{
	hovers.push_back(f);
	return hovers.size();
}

int tmt::ui::ButtonObject::AddClickEvent(std::function<void()> f)
{
	clicks.push_back(f);
	return clicks.size();
}

glm::vec2 tmt::input::Mouse::GetMousePosition()
{
	return mousep;
}

glm::vec2 tmt::input::Mouse::GetMouseDelta()
{
	return mousedelta;
}

std::vector<int> mstates(8);
std::vector<int> kstates;


tmt::input::Mouse::MouseButtonState tmt::input::Mouse::GetMouseButton(int i)
{
	int state = glfwGetMouseButton(renderer->window, i);

	if (state == Press && (mstates[i] == Press || mstates[i] == Hold))
	{
		state = Hold;
	}

	mstates[i] = state;

	return static_cast<MouseButtonState>(state);
}

tmt::input::Keyboard::KeyState tmt::input::Keyboard::GetKey(int key)
{
	int state = glfwGetKey(renderer->window, key);

	if (kstates.size() <= key)
	{
		kstates.resize(key + 1, Release);
		kstates[key] = state;
	}
	else
	{
		if (state == Press && (kstates[key] == Press || kstates[key] == Hold))
		{
			state = Hold;
		}
		kstates[key] = state;
	}

	return static_cast<KeyState>(state);
}

float tmt::time::getTime()
{
	return counterTime;
}

float tmt::time::getSinTime()
{
	return glm::sin(counterTime);
}

float tmt::time::getCosTime()
{
	return glm::cos(counterTime);
}

float tmt::time::getDeltaTime()
{
	return deltaTime;
}

tmt::fs::BinaryReader::BinaryReader(std::streambuf* data) : std::istream(data)
{
	fileSize = tellg();
}

string tmt::fs::StringBinaryReader::fLoadString(u32 offset)
{
	var p = tellg();
	seekg(offset, _Seekbeg);

	u16 size = ReadUInt16();

	string s = ReadString();
	seekg(p, _Seekbeg);

	return s;
}

string tmt::fs::StringBinaryReader::ReadString()
{
	//short size = ReadInt16();
	return ReadUtf8();
}

string tmt::fs::StringBinaryReader::ReadUtf8()
{
	long start = tellg();
	int size = 0;

	while (ReadByte() - 1 > 0 && size < INT_MAX)
	{
		size++;
	}

	seekg(start, _Seekbeg);
	var text = ReadString(size);
	seekg(1, _Seekcur);
	return text;
}

tmt::fs::BinaryReader::ByteOrder tmt::fs::BinaryReader::ReadByteOrder()
{
	byteOrder = BigEndian;
	var bom = Read<ByteOrder>();
	byteOrder = bom;
	return bom;
}

u64 tmt::fs::BinaryReader::ReadUInt64()
{
	return Read<u64>();
}

tmt::fs::StringBinaryReader::StringBinaryReader(string data) : std::stringstream(data)
{
}

u8 tmt::fs::BinaryReader::ReadByte()
{
	return Read<u8>();
}

u16 tmt::fs::BinaryReader::ReadUInt16()
{
	return Read<u16>();
}

u32 tmt::fs::BinaryReader::ReadUInt32()
{
	return Read<u32>();
}

u32 tmt::fs::BinaryReader::ReadOffset()
{
	var offset = static_cast<u32>(ReadUInt64());
	return offset == 0 ? 0 : offset;
}


s16 tmt::fs::BinaryReader::ReadInt16()
{
	return Read<s16>();
}

s64 tmt::fs::BinaryReader::ReadInt64()
{
	return Read<s64>();
}

s32 tmt::fs::BinaryReader::ReadInt32()
{
	return Read<s32>();
}


string tmt::fs::StringBinaryReader::ReadString(int size)
{
	string s = "";

	for (int i = 0; i < size; ++i)
	{
		s += Read<char>();
	}

	return s;
}

btDiscreteDynamicsWorld* dynamicsWorld;
std::vector<btRigidBody*> physicalBodies;
std::vector<btCollisionShape*> collisionObjs;
std::vector<tmt::physics::PhysicsBody*> bodies;
std::vector<tmt::particle::Particle*> managed_particles;
bool doneFirstPhysicsUpdate;

// FIRST TRYYY RAAAAAAAAAAHHHHHH

glm::vec3 convertVec3(btVector3 v)
{
	return glm::vec3{v.x(), v.y(), v.z()};
}

btVector3 convertVec3(glm::vec3 v)
{
	return btVector3(v.x, v.y, v.z);
}


glm::vec3 convertQuatEuler(btQuaternion q)
{
	float x, y, z;

	q.getEulerZYX(x, y, z);

	return degrees(glm::vec3{x, y, z});
}

btQuaternion convertQuat(glm::vec3 q)
{
	float x, y, z;

	var qu = radians(q);

	x = qu.x;
	y = qu.y;
	z = qu.z;

	return btQuaternion(y, x, z);
}


void ApplyTransform(tmt::physics::PhysicsBody* body, btTransform transform)
{
	var parent = body->parent;
	if (body->transRelation == tmt::physics::PhysicsBody::Self)
	{
		var p = body->position;

		body->position = convertVec3(transform.getOrigin());

		//transform.setOrigin(convertVec3(p));
	}
	else
	{
		var p = parent->position;
		var r = parent->rotation;

		parent->position = convertVec3(transform.getOrigin());

		parent->rotation = convertQuatEuler(transform.getRotation());
	}
}

btScalar tmt::physics::CollisionCallback::addSingleResult(btManifoldPoint& cp,
                                                          const btCollisionObjectWrapper* colObj0Wrap,
                                                          int partId0, int index0,
                                                          const btCollisionObjectWrapper* colObj1Wrap, int partId1,
                                                          int index1)
{
	btCollisionObjectWrapper *thisObj, *other;
	int thisIdx, otherIdx;
	int thisPart, otherPart;
	btVector3 thisPos, otherPos;
	btVector3 thisNormal, otherNormal;

	if (colObj0Wrap->getCollisionShape() == collisionObjs[collider->pId])
	{
		thisObj = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
		other = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);

		thisIdx = index0;
		otherIdx = index1;

		thisPart = partId0;
		otherPart = partId1;

		thisPos = cp.getPositionWorldOnA();
		thisNormal = -cp.m_normalWorldOnB;

		otherPos = cp.getPositionWorldOnB();
		otherNormal = cp.m_normalWorldOnB;
	}
	else
	{
		thisObj = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
		other = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);


		thisIdx = index1;
		otherIdx = index0;

		thisPart = partId1;
		otherPart = partId0;

		otherPos = cp.getPositionWorldOnA();
		otherNormal = -cp.m_normalWorldOnB;

		thisPos = cp.getPositionWorldOnB();
		thisPos = cp.m_normalWorldOnB;
	}

	var particle = static_cast<particle::Particle*>(other->getCollisionShape()->getUserPointer());

	if (particle)
	{
		ParticleCollision col{};

		col.contactPoint = convertVec3(thisPos);
		col.normal = convertVec3(thisNormal);
		col.faceId = thisIdx;
		col.other = particle;

		body->OnParticleCollision(col);
	}
	else
	{
		Collision col{};

		col.contactPoint = convertVec3(thisPos);
		col.normal = convertVec3(thisNormal);
		col.faceId = thisIdx;
		col.other = bodies[other->getCollisionShape()->getUserIndex()];

		body->OnCollision(col);
	}

	return 0;
}

tmt::physics::PhysicalWorld::PhysicalWorld()
{
	auto configuration = new btDefaultCollisionConfiguration();

	auto dispatcher = new btCollisionDispatcher(configuration);

	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	auto solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);

	dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
}

bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                     glm::vec3 scale = glm::vec3{1})
{
	glm::vec3 v0 = b - a;
	glm::vec3 v1 = c - a;
	glm::vec3 v2 = p - a;

	float dot00 = glm::dot(v0, v0);
	float dot01 = glm::dot(v0, v1);
	float dot02 = glm::dot(v0, v2);
	float dot11 = glm::dot(v1, v1);
	float dot12 = glm::dot(v1, v2);

	float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	var bl = (u >= 0) && (v >= 0) && (u + v <= 1);

	if (bl)
	{
		//tmt::debug::Gizmos::DrawSphere(a, 1.f);
		//tmt::debug::Gizmos::DrawSphere(b, 1.f);
		//tmt::debug::Gizmos::DrawSphere(c, 1.f);
		//tmt::debug::Gizmos::DrawSphere(p, 1.f);
	}

	return bl;
}

glm::vec3 barycentricCoordinates(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                                 glm::vec3 scale = glm::vec3{1})
{
	glm::vec3 v0 = b - a;
	glm::vec3 v1 = c - a;
	glm::vec3 v2 = p - a;

	float dot00 = glm::dot(v0, v0);
	float dot01 = glm::dot(v0, v1);
	float dot02 = glm::dot(v0, v2);
	float dot11 = glm::dot(v1, v1);
	float dot12 = glm::dot(v1, v2);

	float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
	float w = 1.0f - u - v;

	var bary = glm::vec3{u, v, w};

	//bary -= glm::vec3{ 0.5 };
	//bary = -bary;

	// Debug visualization (optional)
	if (u >= 0 && v >= 0 && w >= 0)
	{
		tmt::debug::Gizmos::DrawSphere(a, 1.f);
		tmt::debug::Gizmos::DrawSphere(b, 1.f);
		tmt::debug::Gizmos::DrawSphere(c, 1.f);
		tmt::debug::Gizmos::DrawSphere(p, 1.f);
		tmt::debug::Gizmos::DrawSphere(bary, 1.f);
	}

	return bary;
}


void tmt::physics::PhysicalWorld::Update()
{
	dynamicsWorld->stepSimulation(1.0 / 6.0f, 1);

	dynamicsWorld->performDiscreteCollisionDetection();
	int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < nManifolds; i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();
		contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());
		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++)
		{
			//std::cout << "Collision between shapes " << obA->getCollisionShape()
			//	<< " and " << obB->getCollisionShape() << std::endl;

			PhysicsBody* goA = nullptr;
			PhysicsBody* goB = nullptr;

			var goai = obA->getCollisionShape()->getUserIndex();
			var gobi = obB->getCollisionShape()->getUserIndex();

			if (goai != -1)
			{
				goA = bodies[goai];
			}
			if (gobi != -1)
			{
				goB = bodies[gobi];
			}

			auto poA = static_cast<particle::Particle*>(obA->getCollisionShape()->getUserPointer());
			auto poB = static_cast<particle::Particle*>(obB->getCollisionShape()->getUserPointer());

			int exo = 0;

			for (int i = 0; i < 4; ++i)
			{
				switch (i)
				{
					case 0:
						if (goA)
							exo++;
						break;
					case 1:
						if (goB)
							exo++;
						break;
					case 2:
						if (poA)
							exo++;
						break;
					case 3:
						if (poB)
							exo++;
						break;
				}

				if (exo >= 2)
					break;
			}

			if (exo == 2)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				btVector3 ptA, ptB;
				ptA = pt.getPositionWorldOnB();
				ptB = pt.getPositionWorldOnA();

				//std::cout << "Collision between " << goA->name << " and "	<< goB->name << std::endl;
				btVector3 nrmB = pt.m_normalWorldOnB;
				btVector3 nrmA = -nrmB;

				//ptA = convertVec3(glm::lerp(convertVec3(ptA), convertVec3(ptB), 0.5f));
				//ptB = ptA;

				var lpta = (obA->getWorldTransform().inverse() * ptA);
				var lptb = (obB->getWorldTransform().inverse() * ptB);
				var faceIdA = -1;
				var faceIdB = -1;

				if (goA)
				{
					var colObj = goA->GetObjectFromType<ColliderObject>();
					if (colObj)
					{
						if (colObj->initInfo.s == Mesh)
						{
							var mesh = colObj->initInfo.mesh;

							glm::vec3 pta = convertVec3(ptA) / colObj->GetGlobalScale();
							//pta = glm::vec3(glm::vec4(pta, 1) * glm::inverse(colObj->GetTransform()));

							for (size_t triIndex = 0; triIndex < mesh->indexCount; triIndex += 3)
							{
								glm::vec3 v0 = mesh->vertices[mesh->indices[triIndex + 0]].position;
								glm::vec3 v1 = mesh->vertices[mesh->indices[triIndex + 1]].position;
								glm::vec3 v2 = mesh->vertices[mesh->indices[triIndex + 2]].position;

								if (pointInTriangle(pta, v0, v1, v2, colObj->GetGlobalScale()))
								{
									faceIdB = triIndex / 3;
									break;
								}
							}
						}
					}
				}

				if (goB)
				{
					var colObj = goB->GetObjectFromType<ColliderObject>();
					if (colObj)
					{
						if (colObj->initInfo.s == Mesh)
						{
							var mesh = colObj->initInfo.mesh;

							var ptb = convertVec3(ptB);
							ptb = glm::vec3(glm::vec4(ptb, 1) * glm::inverse(colObj->GetTransform()));

							var scale = colObj->GetGlobalScale();

							for (size_t triIndex = 0; triIndex < mesh->indexCount; triIndex += 3)
							{
								glm::vec3 v0 = mesh->vertices[mesh->indices[triIndex + 0]].position;
								glm::vec3 v1 = mesh->vertices[mesh->indices[triIndex + 1]].position;
								glm::vec3 v2 = mesh->vertices[mesh->indices[triIndex + 2]].position;
								if (pointInTriangle(ptb, v0, v1, v2, colObj->GetGlobalScale()))
								{
									faceIdA = triIndex / 3;
									break;
								}
							}
						}
					}
				}

				std::function<CollisionBase(bool)> CreateCollisionBase = [this, lpta,ptA,nrmA,lptb,ptB,nrmB,faceIdA,
						faceIdB](bool a) -> CollisionBase
				{
					btVector3 lpt, pta, nrma;
					int fid;
					if (a)
					{
						lpt = lpta;
						pta = ptA;
						nrma = nrmA;
						fid = faceIdA;
					}
					else
					{
						lpt = lptb;
						pta = ptB;
						nrma = nrmB;
						fid = faceIdB;
					}
					return CollisionBase(convertVec3(pta), convertVec3(nrma), fid);
				};

				if (goA && goB)
				{
					ApplyTransform(goA, obA->getWorldTransform());
					ApplyTransform(goB, obB->getWorldTransform());
					{
						auto c = static_cast<Collision>(CreateCollisionBase(true));
						c.other = goB;
						goA->OnCollision(c);
					}
					{
						auto c = static_cast<Collision>(CreateCollisionBase(true));
						c.other = goA;
						goB->OnCollision(c);
					}
				}
				if (goA && poB)
				{
					ApplyTransform(goA, obA->getWorldTransform());
					poB->position = convertVec3(obB->getWorldTransform().getOrigin());
					poB->rotation = convertQuatEuler(obB->getWorldTransform().getRotation());

					{
						auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
						c.other = poB;
						goA->OnParticleCollision(c);
					}
					{
						auto c = static_cast<Collision>(CreateCollisionBase(true));
						c.other = goA;
						poB->OnCollision(c);
					}
				}
				if (goB && poA)
				{
					ApplyTransform(goB, obB->getWorldTransform());

					poA->position = convertVec3(obA->getWorldTransform().getOrigin());
					poA->rotation = convertQuatEuler(obA->getWorldTransform().getRotation());

					{
						auto c = static_cast<Collision>(CreateCollisionBase(true));
						c.other = goB;
						poA->OnCollision(c);
					}
					{
						auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
						c.other = poA;
						goB->OnParticleCollision(c);
					}
				}
				if (poA && poB)
				{
					poA->position = convertVec3(obA->getWorldTransform().getOrigin());
					poA->rotation = convertQuatEuler(obA->getWorldTransform().getRotation());
					poB->position = convertVec3(obB->getWorldTransform().getOrigin());
					poB->rotation = convertQuatEuler(obB->getWorldTransform().getRotation());

					{
						auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
						c.other = poB;
						poA->OnParticleCollision(c);
					}
					{
						auto c = static_cast<ParticleCollision>(CreateCollisionBase(true));
						c.other = poA;
						poB->OnParticleCollision(c);
					}
				}
			}
		}
	}

	doneFirstPhysicsUpdate = true;
}

void tmt::physics::PhysicalWorld::RemoveBody(int pid, int cpid)
{
	dynamicsWorld->removeRigidBody(physicalBodies[pid]);


	physicalBodies.erase(physicalBodies.begin() + pid);
	collisionObjs.erase(collisionObjs.begin() + cpid);

	for (auto body : bodies)
	{
		if (body->pId > pid)
			body->pId--;
		if (body->cPID > cpid)
			body->cPID--;
	}

	for (auto managed_particle : managed_particles)
	{
		if (managed_particle->pId > pid)
		{
			managed_particle->pId--;
		}
		if (managed_particle->cPID > cpid)
		{
			managed_particle->cPID--;
		}
	}
}

std::vector<tmt::physics::PhysicsBody*> tmt::physics::PhysicalWorld::GetGameObjectsCollidingWith(PhysicsBody* collider)
{
	{
		std::vector<PhysicsBody*> collisions;

		int nManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
		for (int i = 0; i < nManifolds; i++)
		{
			btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
			const btCollisionObject* obA = contactManifold->getBody0();
			const btCollisionObject* obB = contactManifold->getBody1();

			auto goA = static_cast<PhysicsBody*>(obA->getCollisionShape()->getUserPointer());
			auto goB = static_cast<PhysicsBody*>(obB->getCollisionShape()->getUserPointer());
			if (goA != nullptr && goB != nullptr)
				std::cout << "Collision between " << goA->name << " and " << goB->name << std::endl;
			if (goA == collider && goB != nullptr && std::find(collisions.begin(), collisions.end(), goB) == collisions.
				end())
				collisions.push_back(goB);
			if (goB == collider && goA != nullptr && std::find(collisions.begin(), collisions.end(), goA) == collisions.
				end())
				collisions.push_back(goA);
		}

		return collisions;

		/*
		std::vector<GameObject*> collisions;
		btCollisionObjectArray coArray = dynamicsWorld->getCollisionObjectArray();
		int nCollisionObjects = coArray.size();
		for (int i = 0; i < nCollisionObjects; i++)
		{
			btCollisionObject* cObj = coArray.at(i);
			if (collider->checkCollideWith(cObj))
				collisions.push_back(static_cast<GameObject*>(cObj->getCollisionShape()->getUserPointer()));
		}
		return collisions;
		*/
	}
}


tmt::physics::PhysicsBody::PhysicsBody(ColliderObject* collisionObj, float mass) : Object()
{
	if (!collisionObj->parent)
	{
		collisionObj->SetParent(this);
	}

	this->mass = mass;
	cPID = collisionObj->pId;

	collisionObjs[cPID]->setUserIndex(bodies.size());

	bodies.push_back(this);
}

void tmt::physics::PhysicsBody::Update()
{
	if (!parent)
		transRelation = Self;

	if (!doneFirstPhysicsUpdate)
	{
		pId = physicalBodies.size();

		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			collisionObjs[cPID]->calculateLocalInertia(mass, localInertia);

		btTransform startTransform;
		startTransform.setIdentity();

		if (transRelation == Self)
		{
			startTransform.setOrigin(convertVec3(position));
			startTransform.setRotation(convertQuat(rotation));
		}
		else
		{
			startTransform.setOrigin(convertVec3(parent->position));
			startTransform.setRotation(convertQuat(parent->rotation));
		}

		auto myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionObjs[cPID], localInertia);

		var rigidBody = new btRigidBody(rbInfo);

		rigidBody->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(rigidBody);

		physicalBodies.push_back(rigidBody);
	}
	else
	{
		//var callback = *callback_;

		//dynamicsWorld->contactTest(physicalBodies[pId].object, callback);
	}

	var pBody = physicalBodies[pId];
	pBody->setUserPointer(this);
	// CONSTRAINTS !!! >x<

	if (pBody && pBody->getMotionState())
	{
		var motionState = pBody->getMotionState();
		btTransform transform;
		motionState->getWorldTransform(transform);
		//transform = pBody->getWorldTransform();

		if (transRelation == Self)
		{
			var p = position;

			position = convertVec3(transform.getOrigin());

			//transform.setOrigin(convertVec3(p));
		}
		else
		{
			var p = parent->position;
			var r = parent->rotation;

			parent->position = convertVec3(transform.getOrigin());

			parent->rotation = convertQuatEuler(transform.getRotation());

			transform.setIdentity();

			transform.setOrigin(convertVec3(p));
			transform.setRotation(convertQuat(r));

			motionState->setWorldTransform(transform);
		}
	}

	//

	//pBody->activate();
}

void tmt::physics::PhysicsBody::SetVelocity(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->setLinearVelocity(convertVec3(v));
}

glm::vec3 tmt::physics::PhysicsBody::GetVelocity()
{
	var pBody = physicalBodies[pId];

	return convertVec3(pBody->getLinearVelocity());
}

void tmt::physics::PhysicsBody::AddImpulse(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->applyCentralImpulse(convertVec3(v));
}

void tmt::physics::PhysicsBody::AddForce(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->applyCentralForce(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetAngular(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->setAngularVelocity(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetLinearFactor(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->setLinearFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetAngularFactor(glm::vec3 v)
{
	var pBody = physicalBodies[pId];

	pBody->setAngularFactor(convertVec3(v));
}

void tmt::physics::PhysicsBody::SetDamping(float linear, float angular)
{
	var pBody = physicalBodies[pId];

	pBody->setDamping(linear, angular);
}

void tmt::physics::PhysicsBody::AddCollisionEvent(std::function<void(Collision)> func)
{
	collisionEvents.push_back(func);
}

void tmt::physics::PhysicsBody::AddParticleCollisionEvent(std::function<void(ParticleCollision)> func)
{
	particleCollisionEvents.push_back(func);
}

glm::vec3 tmt::physics::PhysicsBody::GetBasisColumn(float v)
{
	var pBody = physicalBodies[pId];

	var basis = pBody->getWorldTransform().getBasis();

	var vector = basis.getColumn(v);

	return convertVec3(vector);
}

glm::vec3 tmt::physics::PhysicsBody::GetBasisRow(float v)
{
	var pBody = physicalBodies[pId];

	var basis = pBody->getWorldTransform().getBasis();

	var vector = basis.getRow(v);

	return convertVec3(vector);
}

void tmt::physics::PhysicsBody::OnCollision(Collision c)
{
	for (auto collision_event : collisionEvents)
	{
		collision_event(c);
	}
}

void tmt::physics::PhysicsBody::OnParticleCollision(ParticleCollision c)
{
	for (auto collision_event : particleCollisionEvents)
	{
		collision_event(c);
	}
}

tmt::physics::RaycastHit* tmt::physics::Ray::Cast()
{
	auto start = convertVec3(position);
	auto end = convertVec3(position + (direction * maxDistance));
	btCollisionWorld::ClosestRayResultCallback callback(start, end);

	dynamicsWorld->rayTest(start, end, callback);

	if (callback.hasHit())
	{
		var point = callback.m_hitPointWorld;
		var nrm = callback.m_hitNormalWorld;

		var obj = callback.m_collisionObject;

		auto hit = new RaycastHit;

		hit->point = convertVec3(point);
		hit->normal = convertVec3(nrm);


		PhysicsBody* goA = nullptr;

		var goai = obj->getCollisionShape()->getUserIndex();

		if (goai != -1)
		{
			goA = bodies[goai];
		}

		hit->hit = goA;

		return hit;
	}
	return nullptr;
}

btCollisionShape* ShapeFromInfo(tmt::physics::ColliderInitInfo i)
{
	btCollisionShape* shape;
	switch (i.s)
	{
		case tmt::physics::Box:
			shape = new btBoxShape(convertVec3(i.bounds / 2.f));
			break;
		case tmt::physics::Sphere:
			shape = new btSphereShape(i.radius);
			break;
		case tmt::physics::Capsule:
			shape = new btCapsuleShape(i.radius, i.height);
			break;
		case tmt::physics::Mesh:
			{
				auto indices = new int[i.mesh->indexCount];

				for (int j = 0; j < i.mesh->indexCount; ++j)
				{
					indices[j] = i.mesh->indices[j];
				}

				auto vertices = new glm::vec3[i.mesh->vertexCount];

				for (int j = 0; j < i.mesh->vertexCount; ++j)
				{
					vertices[j] = i.mesh->vertices[j].position;
				}

				auto indexVertexArray = new btTriangleIndexVertexArray(
					i.mesh->indexCount / 3, // Number of triangles
					&indices[0], // Pointer to index array
					sizeof(int) * 3, // Stride between indices
					i.mesh->vertexCount, // Number of vertices
					reinterpret_cast<float*>(&vertices[0]), // Pointer to vertex array
					sizeof(glm::vec3) // Stride between vertices (sizeof(float) * 3)
				);

				shape = new btBvhTriangleMeshShape(indexVertexArray, true);

				//shape = new btBvhTriangleMeshShape(triangleMesh, true);
			}
			break;
		default:
			break;
	}

	return shape;
}

tmt::physics::ColliderObject::ColliderObject(ColliderInitInfo i, Object* parent)
{
	SetParent(parent);
	initInfo = i;
	var shape = ShapeFromInfo(i);
	if (i.s == Mesh)
	{
		var scale = convertVec3(GetGlobalScale());
		shape->setLocalScaling(scale);
	}

	shape->setUserIndex(-1);


	pId = collisionObjs.size();
	collisionObjs.push_back(shape);
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForBox(glm::vec3 bounds)
{
	var info = ColliderInitInfo();

	info.bounds = bounds;
	info.s = Box;

	return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForSphere(float radius)
{
	var info = ColliderInitInfo();

	info.radius = radius;
	info.s = Sphere;

	return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForCapsule(float radius, float height)
{
	var info = ColliderInitInfo();

	info.radius = radius;
	info.height = height;
	info.s = Capsule;

	return info;
}

tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForMesh(render::Mesh* mesh)
{
	var info = ColliderInitInfo();

	info.mesh = mesh;
	info.s = Mesh;

	return info;
}

glm::mat4 tmt::particle::Particle::getTransform()
{
	var p = position;
	var r = rotation;
	var s = scale;

	if (pId == -1)
	{
		p += emitterParent->GetGlobalPosition();
		p += emitterParent->GetGlobalRotation();
	}

	var rx = rotate(glm::mat4(1.0), glm::radians(r.x), {1, 0, 0});
	var ry = rotate(glm::mat4(1.0), glm::radians(r.y), {0, 1, 0});
	var rz = rotate(glm::mat4(1.0), glm::radians(r.z), {0, 0, 1});

	var rt = ry * rx * rz;

	return translate(glm::mat4(1.0), p) * rt * glm::scale(glm::mat4(1.0), s);
}

glm::vec3 tmt::particle::Particle::GetUp()
{
	auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

	return q * glm::vec3{0, 1, 0};
}

glm::vec3 tmt::particle::Particle::GetRight()
{
	auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

	return q * glm::vec3{1, 0, 0};
}

glm::vec3 tmt::particle::Particle::GetForward()
{
	auto q = glm::quat(radians(rotation + emitterParent->GetGlobalRotation()));

	return q * glm::vec3{0, 0, 1};
}

void tmt::particle::Particle::OnCollision(physics::Collision c)
{
	emitterParent->OnCollision(c, this);
}

void tmt::particle::Particle::OnParticleCollision(physics::ParticleCollision c)
{
}

tmt::particle::ParticleEmitter::ParticleEmitter() : Object()
{
	system = new ParticleSystem();

	system->renderer.material = new render::Material(defaultShader);
}

void tmt::particle::ParticleEmitter::Emit(int amount)
{
	if (particles.size() >= system->maxParticles)
		return;


	for (int i = 0; i < amount; ++i)
	{
		auto particle = new Particle();

		particle->lifetime = system->startLifetime;
		particle->scale = glm::vec3{system->startSize};

		particle->position = {0, 0, 0};
		particle->rotation = {0, 0, 0};
		particle->emitterParent = this;
		particle->velocity = particle->GetUp();

		switch (system->shape.type)
		{
			case ParticleSystem::SystemShape::Cone:
				{
					float dir = randval(0, 360);
					float rad = randomFloat(0, system->shape.radius);

					//particle->position += glm::vec3{ glm::sin(dir) * rad,0,glm::cos(dir) * rad };
				}
				break;
		}

		if (system->collision.useColliders)
		{
			particle->pId = physicalBodies.size();

			physics::ColliderInitInfo info{
				system->collision.shape, particle->scale, particle->scale.x, particle->scale.y
			};

			var shape = ShapeFromInfo(info);

			shape->setUserPointer(particle);

			particle->cPID = collisionObjs.size();
			collisionObjs.push_back(shape);

			float mass = system->collision.mass;

			bool isDynamic = true;

			btVector3 localInertia(0, 0, 0);
			if (isDynamic)
				collisionObjs[particle->cPID]->calculateLocalInertia(mass, localInertia);

			btTransform startTransform;
			startTransform.setIdentity();
			startTransform.setOrigin(convertVec3(particle->position + GetGlobalPosition()));
			startTransform.setRotation(convertQuat(particle->rotation + GetGlobalRotation()));

			auto myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionObjs[particle->cPID],
			                                                localInertia);

			var rigidBody = new btRigidBody(rbInfo);

			//rigidBody->setActivationState(DISABLE_DEACTIVATION);

			rigidBody->setUserPointer(particle);

			dynamicsWorld->addRigidBody(rigidBody);

			physicalBodies.push_back(rigidBody);

			managed_particles.push_back(particle);

			rigidBody->applyCentralImpulse(convertVec3(particle->GetUp() * system->startSpeed));
		}
		else
		{
			particle->pId = -1;
		}


		particles.push_back(particle);
	}
}

void tmt::particle::ParticleEmitter::Update()
{
	float timeScale = 0.1f;

	if (time::getTime() <= 1)
	{
		isPlaying = system->playOnStart;
	}

	if (isPlaying)
	{
		time += time::getDeltaTime() * timeScale;
		time_alloc += time::getDeltaTime() * timeScale;

		if (time_alloc >= timeScale && system->emission.rateOverTime > 0)
		{
			Emit();
			time_alloc = 0;
		}

		if (time >= system->duration)
		{
			time = 0;
			if (!system->looping)
				isPlaying = false;
		}
	}

	std::vector<Particle*> deleteParticles;
	for (auto particle : particles)
	{
		if (system->collision.useColliders == false)
			particle->pId = -1;

		if (isPlaying)
		{
			particle->lifetime -= time::getDeltaTime();

			if (particle->pId == -1)
			{
				particle->position += particle->velocity * system->startSpeed * time::getDeltaTime();
			}
			else
			{
				var body = physicalBodies[particle->pId];
				particle->position = convertVec3(body->getWorldTransform().getOrigin());
				particle->rotation = convertQuatEuler(body->getWorldTransform().getRotation());
				particle->velocity = convertVec3(body->getLinearVelocity());
			}
		}

		if (particle->lifetime <= 0)
		{
			deleteParticles.push_back(particle);
		}
		else
		{
			system->renderer.mesh->draw(particle->getTransform(), system->renderer.material);
		}
	}

	for (auto deleteParticle : deleteParticles)
	{
		particles.erase(std::find(particles.begin(), particles.end(), deleteParticle));
		managed_particles.erase(std::find(managed_particles.begin(), managed_particles.end(), deleteParticle));
		mainScene->physicsWorld->RemoveBody(deleteParticle->pId, deleteParticle->cPID);
	}
}

void tmt::particle::ParticleEmitter::OnCollision(physics::Collision c, Particle* p)
{
	p->lifetime -= system->collision.lifetimeLoss * system->startLifetime;
}

void tmt::debug::Gizmos::DrawLine(glm::vec3 start, glm::vec3 end)
{
	var clr = color;

	debugCalls.push_back(DebugCall{Line, start, end, 0, clr});
}

void tmt::debug::Gizmos::DrawSphere(glm::vec3 position, float radius)
{
	var clr = color;

	debugCalls.push_back(DebugCall{Sphere, position, glm::vec3{0}, radius, clr});
}
