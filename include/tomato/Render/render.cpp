#include "render.hpp" 
#include "globals.hpp"
#include "vertex.h"

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
    case bgfx::UniformType::Mat4: {
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

tmt::render::SubShader::SubShader(string name, ShaderType type)
{
    string shaderPath = "";

    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Noop:
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12:
        shaderPath = "runtime/shaders/dx/";
        break;
    case bgfx::RendererType::Gnm:
        shaderPath = "shaders/pssl/";
        break;
    case bgfx::RendererType::Metal:
        shaderPath = "shaders/metal/";
        break;
    case bgfx::RendererType::OpenGL:
        shaderPath = "runtime/shaders/gl/";
        break;
    case bgfx::RendererType::OpenGLES:
        shaderPath = "shaders/essl/";
        break;
    case bgfx::RendererType::Vulkan:
        shaderPath = "runtime/shaders/spirv/";
        break;
    // case bgfx::RendererType::Nvn:
    // case bgfx::RendererType::WebGPU:
    case bgfx::RendererType::Count:
        handle = BGFX_INVALID_HANDLE;
        return; // count included to keep compiler warnings happy
    }

    shaderPath += name;

    switch (type)
    {
    case Vertex:
        shaderPath += ".cvbsh";
        break;
    case Fragment:
        shaderPath += ".cfbsh";
        break;
    case Compute:
        shaderPath += ".ccbsh";
        break;
        // default: shaderPath += ".cbsh"; break;
    }

    std::ifstream in(shaderPath, std::ifstream::ate | std::ifstream::binary);

    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    const bgfx::Memory *mem = bgfx::alloc(size);
    in.read(reinterpret_cast<char *>(mem->data), size);

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

tmt::render::ShaderUniform *tmt::render::SubShader::GetUniform(string name)
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

void tmt::render::Shader::Push(int viewId, MaterialOverride **overrides, size_t oc)
{
    std::map<std::string, MaterialOverride> m_overrides;

    if (overrides != nullptr)
    {
        for (int i = 0; i < oc; ++i)
        {
            var name = overrides[i]->name;

            var pair = std::make_pair(name, *overrides[i]);
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

tmt::render::ComputeShader::ComputeShader(SubShader *shader)
{
    program = createProgram(shader->handle, true);
    internalShader = shader;
}

void tmt::render::ComputeShader::SetUniform(string name, bgfx::UniformType::Enum type, const void *data)
{
    var uni = createUniform(name.c_str(), type);

    setUniform(uni, data);

    destroy(uni);
}

void tmt::render::ComputeShader::SetMat4(string name, glm::mat4 m)
{
    // internalShader->GetUniform()
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

tmt::render::MaterialOverride *tmt::render::Material::GetUniform(string name)
{
    for (auto override : overrides)
        if (override->name == name)
            return override;

    return nullptr;
}

u64 tmt::render::Material::GetMaterialState()
{
    u64 v = state.cull;
    v |= state.depth;
    v |= BGFX_STATE_WRITE_MASK;

    return v;
}

tmt::render::Material::Material(Shader *shader)
{
    Reload(shader);
}

void tmt::render::Material::Reload(Shader *shader)
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

void tmt::render::Mesh::draw(glm::mat4 transform, Material *material)
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
    if (path.ends_with(".tmdl"))
    {
        var reader = new fs::BinaryReader(path);

        var tmdlSig = reader->ReadString(4);

        if (tmdlSig != "TMDL")
        {
            std::cout << "Incorrect tmdl format!" << std::endl;
            return;
        }

        var meshCount = reader->ReadInt32();

        for (int i = 0; i < meshCount; ++i)
        {
            var tmshSig = reader->ReadString(4);
            if (tmshSig == "TMSH")
            {
                var vtxCount = reader->ReadInt32();
                var vertices = new Vertex[vtxCount];

                reader->Skip(4);

                for (int i = 0; i < vtxCount; ++i)
                {
                    glm::vec3 p = math::convertVec3(reader->ReadFloatArray(3));
                    glm::vec3 n = math::convertVec3(reader->ReadFloatArray(3));
                    glm::vec2 u = math::convertVec2(reader->ReadFloatArray(2));

                    Vertex vtx = {p, n, u};
                    vertices[i] = vtx;
                }

                reader->Skip(4);
                var idxCount = reader->ReadInt32();
                var incs = new u16[idxCount];

                for (int i = 0; i < idxCount; ++i)
                {
                    incs[i] = reader->ReadUInt16();
                }

                var materialIndex = reader->ReadInt32();

                meshes.push_back(createMesh(vertices, incs, vtxCount, idxCount, Vertex::getVertexLayout()));
                materialIndices.push_back((materialIndex));
            }
        }

    }
    else
    {


        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                   aiProcess_FindInvalidData | aiProcess_PreTransformVertices);

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

            Vertex* verts = new Vertex[vertices.size()];
            std::copy(vertices.begin(), vertices.end(), verts);

            u16* incs = new u16[indices.size()];
            std::copy(indices.begin(), indices.end(), incs);

            meshes.push_back(createMesh(verts, incs, vertices.size(), indices.size(), Vertex::getVertexLayout()));
            materialIndices.push_back(msh->mMaterialIndex);
        }
    }
}

tmt::render::Model::Model(const aiScene *scene)
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

        meshes.push_back(
            createMesh(vertices.data(), indices.data(), vertices.size(), indices.size(), Vertex::getVertexLayout()));
        materialIndices.push_back(msh->mMaterialIndex);
    }
}

tmt::render::Texture::Texture(string path)
{
    int nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    uint64_t textureFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_POINT; // Adjust as needed
    bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA8;

    // Create the texture in bgfx, passing the image data directly
    handle = createTexture2D(static_cast<uint16_t>(width), static_cast<uint16_t>(height),
                             false, // no mip-maps
                             1,     // single layer
                             textureFormat, textureFlags,
                             bgfx::copy(data, width * height * nrChannels) // copies the image data
    );

    stbi_image_free(data);

    format = textureFormat;
}

tmt::render::Texture::Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory *mem)
{
    handle = createTexture2D(width, height, false, 1, tf, flags, mem);

    format = tf;
    this->width = width;
    this->height = height;
}

tmt::render::RenderTexture::RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 cf)
{
    const bgfx::Memory *mem = nullptr;
    if (format == bgfx::TextureFormat::RGBA8)
    {
        // std::vector<GLubyte> pixels(width * height * 4, (GLubyte)0xffffffff);
        // mem = bgfx::copy(pixels.data(), width * height* 4);
    }

    realTexture = new Texture(width, height, format,
                              BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT |
                                  BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
                              mem);

    handle = createFrameBuffer(1, &realTexture->handle, false);
    this->format = format;

    bgfx::setViewName(vid, "RenderTexture");
    bgfx::setViewClear(vid, cf);
    bgfx::setViewRect(vid, 0, 0, width, height);
    setViewFrameBuffer(vid, handle);
}

float *tmt::render::Camera::GetView()
{
    var Front = GetFront();
    var Up = GetUp();

    float view[16];

    mtxLookAt(view, bx::Vec3(position.x, position.y, position.z), math::convertVec3(position + Front),
              bx::Vec3(Up.x, Up.y, Up.z));

    return view;
}

float *tmt::render::Camera::GetProjection()
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
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower
    // movement.
    glm::vec3 Up = normalize(cross(Right, Front));

    return Up;
}

tmt::render::Camera *tmt::render::Camera::GetMainCamera()
{
    return mainCamera;
}

tmt::render::Camera::Camera()
{
    mainCamera = this;
}

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

tmt::render::Mesh *tmt::render::createMesh(Vertex *data, u16 *indices, u32 vertCount, u32 triSize,
                                           bgfx::VertexLayout layout)
{
    u16 stride = layout.getStride();
    u16 vertS = stride * vertCount;
    u16 indeS = sizeof(u16) * triSize;

    var mesh = new Mesh();
    //memcpy(mesh->vertices, data, sizeof(data));
    //memcpy(mesh->indices, indices, sizeof(indices));
    mesh->indices = indices;
    mesh->vertices = data;

    {
        const bgfx::Memory *mem = bgfx::alloc(vertS);

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

            const bgfx::Memory *vmem = bgfx::alloc(vsz);

            bx::memCopy(vmem->data, positions, vsz);

            bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(vmem, playout, flags);

            mesh->vertexBuffers.push_back(vertexBuffer);
        }

        {
            var vsz = sizeof(glm::vec3) * vertCount;

            const bgfx::Memory *vmem = bgfx::alloc(vsz);

            bx::memCopy(vmem->data, normals, vsz);

            bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(vmem, nlayout, flags);

            mesh->vertexBuffers.push_back(vertexBuffer);
        }
        {
            var vsz = sizeof(glm::vec2) * vertCount;

            const bgfx::Memory *vmem = bgfx::alloc(vsz);

            bx::memCopy(vmem->data, uvs, vsz);

            bgfx::DynamicVertexBufferHandle vertexBuffer = createDynamicVertexBuffer(vmem, ulayout, flags);

            mesh->vertexBuffers.push_back(vertexBuffer);
        }
    }

    {
        const bgfx::Memory *mem = bgfx::alloc(indeS);

        bx::memCopy(mem->data, indices, indeS);

        bgfx::IndexBufferHandle ibh = createIndexBuffer(mem, BGFX_BUFFER_COMPUTE_READ);

        mesh->ibh = ibh;
        mesh->indexCount = triSize;

        const bgfx::Memory *vmem = bgfx::alloc(indeS);

        bx::memCopy(vmem->data, indices, indeS);

        mesh->indexBuffer = createDynamicIndexBuffer(vmem, BGFX_BUFFER_COMPUTE_READ);
    }

    return mesh;
}

void tmt::render::pushDrawCall(DrawCall d)
{
    calls.push_back(d);
}

tmt::render::RendererInfo *tmt::render::init()
{
    glfwSetErrorCallback(glfw_errorCallback);
    if (!glfwInit())
        return nullptr;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(1024, 768, "helloworld", nullptr, nullptr);
    if (!window)
        return nullptr;
    // glfwSetKeyCallback(window, glfw_keyCallback);
    //  Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    //  Most graphics APIs must be used on the same thread that created the window.
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
    // init.debug = true;

    init.vendorId = BGFX_PCI_ID_NVIDIA;

    // init.type = bgfx::RendererType::OpenGL;

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

    const bgfx::Stats *stats = bgfx::getStats();

    //bgfx::dbgTextPrintf(5, 5, 0x0f, "%s", getRendererName(bgfx::getRendererType()));

    for (auto d : debugCalls)
    {
        switch (d.type)
        {
            case debug::Text:
            {
                    bgfx::dbgTextPrintf(d.origin.x, d.origin.y, 0x4f, "%s.", d.text.c_str());
            }
                break;
            default:
                break;
                ;
        }
    }

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

    float t[4] = {static_cast<float>(counterTime), static_cast<float>(glm::sin(counterTime)),
                  static_cast<float>(glm::cos(counterTime)), 0};
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
                // bgfx::setViewTransform(0, mainCamera->GetView(), oneMat);
                break;
            case MaterialState::Proj:
                // bgfx::setViewTransform(0, oneMat, proj);
                break;
            case MaterialState::None:
                // bgfx::setViewTransform(0, oneMat, oneMat);
                break;
            case MaterialState::ViewOrthoProj:
                // bgfx::setViewTransform(0, mainCamera->GetView(), ortho);
                setUniform(orthoHandle, ortho);
                break;
            case MaterialState::OrthoProj:
                // bgfx::setViewTransform(0, oneMat, ortho);
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

        // bgfx::discard();
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
        case debug::Line: {
            dde.push();

            dde.moveTo(math::convertVec3(d.origin));
            dde.lineTo(math::convertVec3(d.direction));

            dde.pop();
        }
        break;
        case debug::Sphere: {
            dde.push();

            // dde.moveTo(math::convertVec3(d.origin));
            dde.setWireframe(false);
            dde.drawOrb(d.origin.x, d.origin.y, d.origin.z, d.radius);

            dde.pop();
        }
            break;
            default:
            break;;
        }
    }

    dde.end();

    debugCalls.clear();

    frameTime = bgfx::frame();
    glfwPollEvents();
    counterTime++;

    bgfx::touch(0);
    // bgfx::touch(1);
}

void tmt::render::shutdown()
{
    bgfx::shutdown();
    glfwTerminate();
}
