#include "render.hpp" 
#include "globals.cpp" 

tmt::render::Mesh *tmt::render::createMesh(Vertex *data, u16 *indices, u32 vertCount, u32 triSize,
                                           bgfx::VertexLayout layout)
{
    u16 stride = layout.getStride();
    u16 vertS = stride * vertCount;
    u16 indeS = sizeof(u16) * triSize;

    var mesh = new Mesh();
    mesh->vertices = data;
    mesh->indices = indices;

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
        case debug::Gizmos::Line: {
            dde.push();

            dde.moveTo(math::convertVec3(d.origin));
            dde.lineTo(math::convertVec3(d.direction));

            dde.pop();
        }
        break;
        case debug::Gizmos::Sphere: {
            dde.push();

            // dde.moveTo(math::convertVec3(d.origin));
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
    // bgfx::touch(1);
}

void tmt::render::shutdown()
{
    bgfx::shutdown();
    glfwTerminate();
}
