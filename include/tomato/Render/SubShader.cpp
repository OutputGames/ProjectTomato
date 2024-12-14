#include "SubShader.hpp" 
#include "globals.cpp" 

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

