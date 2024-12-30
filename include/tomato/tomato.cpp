#include "tomato.hpp"
#include "bx/math.h"
#include "bimg/include/bimg/bimg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void glfw_errorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static tmt::render::RendererInfo* renderer;
std::vector<tmt::render::DrawCall> calls;


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

	switch (bgfx::getRendererType()) {
	case bgfx::RendererType::Noop:
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12:    shaderPath = "runtime/shaders/dx/";  break;
	//case bgfx::RendererType::Gnm:           shaderPath = "shaders/pssl/";  break;
	//case bgfx::RendererType::Metal:         shaderPath = "shaders/metal/"; break;
	case bgfx::RendererType::OpenGL:        shaderPath = "runtime/shaders/gl/";  break;
	//case bgfx::RendererType::OpenGLES:      shaderPath = "shaders/essl/";  break;
	case bgfx::RendererType::Vulkan:        shaderPath = "runtime/shaders/spirv/"; break;
	//case bgfx::RendererType::Nvn:
	//case bgfx::RendererType::WebGPU:
	case bgfx::RendererType::Count: handle = BGFX_INVALID_HANDLE;
		return; // count included to keep compiler warnings happy
	}

	shaderPath += name;

	switch (type)
	{
		case Vertex: shaderPath += ".cvbsh"; break;
		case Fragment: shaderPath += ".cfbsh"; break;
		case Compute: shaderPath += ".ccbsh"; break;
		//default: shaderPath += ".cbsh"; break;
	}

	std::ifstream in(shaderPath, std::ifstream::ate | std::ifstream::binary);

	in.seekg(0, std::ios::end);
	std::streamsize size = in.tellg();
	in.seekg(0, std::ios::beg);

	const bgfx::Memory* mem = bgfx::alloc(size);
	in.read(reinterpret_cast<char*>(mem->data), size);

	in.close();
	
	handle = bgfx::createShader(mem);

	var unis = std::vector<bgfx::UniformHandle>();
	var uniformCount = bgfx::getShaderUniforms(handle);
	unis.resize(uniformCount);
	bgfx::getShaderUniforms(handle, unis.data(), uniformCount);

	std::vector<string> uniNames;

	for (int i = 0; i < uniformCount; i++)
	{
		bgfx::UniformInfo info = {};

		bgfx::getUniformInfo(unis[i], info);

		if (std::find(uniNames.begin(), uniNames.end(), info.name) == uniNames.end()) {

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
	for each (var uni in uniforms)
	{
		if (uni->name == name)
			return uni;
	}

	return {};
}

tmt::render::Shader::Shader(ShaderInitInfo info)
{
	program = bgfx::createProgram(info.vertexProgram->handle, info.fragmentProgram->handle);

	subShaders.push_back(info.vertexProgram);
	subShaders.push_back(info.fragmentProgram);
}

void tmt::render::Shader::Push(MaterialOverride** overrides, size_t oc)
{
	std::map<std::string, MaterialOverride> m_overrides;

	if (overrides != nullptr) {
		for (int i = 0; i < oc; ++i)
		{

			var name = overrides[i]->name;

			var pair = std::make_pair<std::string, MaterialOverride>((std::string)name, (MaterialOverride)*overrides[i]);
			m_overrides.insert(pair);
		}
	}

	for each (var shader in subShaders)
	{
		for each (var uni in shader->uniforms)
		{

			if (overrides != nullptr) {
				if (m_overrides.count(uni->name))
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


	bgfx::submit(0, program);
}


tmt::render::MaterialOverride* tmt::render::Material::GetUniform(string name)
{
	for (auto override : overrides)
		if (override->name == name) return override;

	return nullptr;
}

u64 tmt::render::Material::GetMaterialState()
{
	return state.cull | state.depth | state.write | BGFX_STATE_MSAA;
}

tmt::render::Material::Material(Shader* shader)
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

void tmt::render::Mesh::draw(glm::mat4 transform, Material* material)
{
	var drawCall = DrawCall();

	drawCall.mesh = this;
	drawCall.state = material->GetMaterialState();

	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			drawCall.transformMatrix[x][y] = transform[x][y];
		}
	}

	drawCall.program = material->shader;
	if (material->overrides.size() > 0) {
		drawCall.overrides = material->overrides.data();
		drawCall.overrideCt = material->overrides.size();
	}
	else
		drawCall.overrides = nullptr;

	pushDrawCall(drawCall);
}

tmt::render::Mesh* tmt::render::createMesh(Vertex* data, u16* indices, u32 vertCount, u32 triSize, bgfx::VertexLayout layout)
{
	u16 stride = layout.getStride();
	u16 vertS = stride * vertCount;
	u16 indeS = sizeof(u16) * triSize;

	var mesh = new Mesh();

	{

		const bgfx::Memory* mem = bgfx::alloc(vertS);

		bx::memCopy(mem->data, data, vertS);

		bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(mem, layout);
		mesh->vbh = vbh;
	}

	{

		const bgfx::Memory* mem = bgfx::alloc(indeS);

		bx::memCopy(mem->data, indices, indeS);

		bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(mem);

		mesh->ibh = ibh;

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
	init.resolution.width = (uint32_t)width;
	init.resolution.height = (uint32_t)height;
	init.resolution.reset = BGFX_RESET_VSYNC;
	if (!bgfx::init(init))
		return nullptr;
	// Set view 0 to the same dimensions as the window and to clear the color buffer.
	const bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
	bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

	var m_debug = BGFX_DEBUG_TEXT;

	bgfx::setDebug(m_debug);



	renderer = new tmt::render::RendererInfo();
	calls = std::vector<DrawCall>();

	renderer->clearView = kClearView;
	renderer->window = window;

	renderer->windowWidth = width;
	renderer->windowHeight = height;

    return renderer;
}

static int counter = 0;

void tmt::render::update()
{

	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, uint16_t(renderer->windowWidth), uint16_t(renderer->windowHeight));

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(0);
	bgfx::dbgTextClear();

	const bgfx::Stats* stats = bgfx::getStats();

	bgfx::dbgTextPrintf(5, 5, 0x0f, "%s", bgfx::getRendererName(bgfx::getRendererType()));

	bgfx::setDebug(BGFX_DEBUG_TEXT);

	var camp = bx::Vec3{ glm::sin(counter*0.01f),0,glm::cos(counter*0.01f) };
	var camt = bx::Vec3{ 0,0,0 };
	var camu = bx::Vec3{ 0,1,0 };

	float view[16];
	float proj[16];

	bx::mtxLookAt(view, camp, camt, camu);
	bx::mtxProj(proj, 90.0f, flt renderer->windowWidth / flt renderer->windowHeight, 0.01, 100.0f, bgfx::getCaps()->homogeneousDepth);

	bgfx::setViewTransform(0, view,proj);

	uint64_t state = 0
		| BGFX_STATE_WRITE_R
		| BGFX_STATE_WRITE_G
		| BGFX_STATE_WRITE_B
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CW
		| BGFX_STATE_MSAA;

	for (auto call : calls)
	{

		bgfx::setTransform(call.transformMatrix);

		bgfx::setVertexBuffer(0, call.mesh->vbh);
		bgfx::setIndexBuffer(call.mesh->ibh);

		bgfx::setState(call.state);

		call.program->Push(call.overrides, call.overrideCt);

		bgfx::discard();
	}
	calls.clear();

	bgfx::frame();
	glfwPollEvents();
	counter++;
}

void tmt::render::shutdown()
{
	bgfx::shutdown();
	glfwTerminate();
}

::tmt::engine::EngineInfo* ::tmt::engine::init()
{
	var rendererInfo = tmt::render::init();

	var engineInfo = new EngineInfo();
	engineInfo->renderer = rendererInfo;

	return engineInfo;
}

tmt::render::Texture::Texture(string path)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);


	uint64_t textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;  // Adjust as needed
	bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA8;

	// Create the texture in bgfx, passing the image data directly
	handle = bgfx::createTexture2D(
		static_cast<uint16_t>(width),
		static_cast<uint16_t>(height),
		false,       // no mip-maps
		1,           // single layer
		textureFormat,
		textureFlags,
		bgfx::copy(data, width * height * nrChannels)  // copies the image data
	);

	stbi_image_free(data);
}

void tmt::render::ShaderUniform::Use()
{
	switch (type)
	{
	case bgfx::UniformType::Sampler:
		if (tex) {
			bgfx::setTexture(0, handle, tex->handle);
		}
		break;
	case bgfx::UniformType::End:
		break;
	case bgfx::UniformType::Vec4:
		bgfx::setUniform(handle, tmt::math::vec4toArray(v4));
		break;
	case bgfx::UniformType::Mat3:
		bgfx::setUniform(handle, tmt::math::mat3ToArray(m3));
		break;
	case bgfx::UniformType::Mat4:
		bgfx::setUniform(handle, tmt::math::mat4ToArray(m4));
		break;
	case bgfx::UniformType::Count:
		break;
	default:
		break;
	}
}

float* tmt::math::vec4toArray(glm::vec4 v)
{
	float f[4] = { v.x,v.y,v.z,v.w };
	return f;
}

float** tmt::math::mat4ToArray(glm::mat4 m)
{
	return nullptr;
}

float** tmt::math::mat3ToArray(glm::mat3 m)
{
	return nullptr;
}

void tmt::obj::Object::Update()
{
	for (auto child : children) {

		child->Update();
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
	var m = glm::mat4(1.0);

	m = glm::translate(m, GetGlobalPosition());

	var r = GetGlobalRotation();
	m = glm::rotate(m, r.x, { 1,0,0 });
	m = glm::rotate(m, r.y, { 0,1,0 });
	m = glm::rotate(m, r.z, { 0,0,1 });

	m = glm::scale(m, GetGlobalScale());

	return m;
}

void tmt::obj::MeshObject::Update()
{
	if (mesh != nullptr && material != nullptr)
		mesh->draw(GetTransform(), material);

	Object::Update();
}
