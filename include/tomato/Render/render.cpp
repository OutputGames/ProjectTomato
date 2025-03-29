#include "render.hpp"
#include "globals.hpp"
#include "vertex.h"

#include <ft2build.h>
#include <bx/timer.h>


#include FT_FREETYPE_H

#define ResMgr tmt::fs::ResourceManager::pInstance

using namespace tmt::render;

Color Color::White = {1, 1, 1, 1};
Color Color::Blue = {0, 0, 1, 1};
Color Color::Green = {0, 1, 0, 1};
Color Color::Red = {1, 0, 0, 1};
Color Color::Black = {0, 0, 0, 1};
Color Color::Gray = {0.5, 0.5, 0.5, 1};

const void* getTimeUniform()
{

    float t[4] = {static_cast<float>(counterTime), static_cast<float>(glm::sin(counterTime)),
                  static_cast<float>(glm::cos(counterTime)), static_cast<float>(renderer->usePosAnim)};

    return t;
}

RendererInfo* RendererInfo::GetRendererInfo()
{
    return renderer;
}

void ShaderUniform::Use(SubShader* shader)
{


    switch (type)
    {
        case tmgl::UniformType::Sampler:
        {
            Texture* t = tex;

            if (tex)
            {
                t = tex;
            }

            if (!tex || tex->name == "White")
            {
                t = fs::ResourceManager::pInstance->loaded_textures["White"];
            }
            else if (tex && tex->name != "White")
            {
                //t = fs::ResourceManager::pInstance->loaded_textures["White"];
            }

            //t = fs::ResourceManager::pInstance->loaded_textures["White"];

            if (t == nullptr)
                return;

            var texSets = shader->texSets;
            int texSet = 0;
            for (auto set : texSets)
            {
                if (set == name)
                    break;

                texSet++;
            }


            if (forcedSamplerIndex > -1)
            {
                texSet = forcedSamplerIndex;
            }

            setTexture(texSet, handle, t->handle);
        }
        break;
        case tmgl::UniformType::End:
            break;
        case tmgl::UniformType::Vec4:
            setUniform(handle, value_ptr(v4));
            break;
        case tmgl::UniformType::Mat3:
            setUniform(handle, value_ptr(m3));
            break;
        case tmgl::UniformType::Mat4:
        {
            setUniform(handle, value_ptr(m4));
        }
        break;
        case tmgl::UniformType::Count:
            break;
        default:
            break;
    }
}

ShaderUniform::~ShaderUniform()
{
    destroy(handle);
}

SubShader::SubShader(string name, ShaderType type)
{

    this->name = name;
    this->type = type;
    Reload();

    fs::ResourceManager::pInstance->loaded_sub_shaders[name] = this;
}

ShaderUniform* SubShader::GetUniform(string name, bool force)
{
    for (var uni : uniforms)
    {
        if (uni->name == name)
            return uni;
    }

    if (force)
    {
        var ovr = new ShaderUniform();
        ovr->name = name;

        return ovr;
    }

    return {};
}

void SubShader::Reload()
{
    if (isLoaded && isValid(handle))
    {
        destroy(handle);
        uniforms.clear();
    }
    isLoaded = true;

    string shaderPath = "";

    switch (tmgl::getRendererType())
    {
        case tmgl::RendererType::Noop:
        case tmgl::RendererType::Direct3D11:
        case tmgl::RendererType::Direct3D12:
            shaderPath = "runtime/shaders/dx/";
            break;
        case tmgl::RendererType::OpenGL:
            shaderPath = "runtime/shaders/gl/";
            break;
        case tmgl::RendererType::Vulkan:
            shaderPath = "runtime/shaders/spirv/";
            break;
        // case tmgl::RendererType::Nvn:
        // case tmgl::RendererType::WebGPU:
        case tmgl::RendererType::Count:
            handle = TMGL_INVALID_HANDLE;
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

    const tmgl::Memory* mem = tmgl::alloc(size);
    in.read(reinterpret_cast<char*>(mem->data), size);

    in.close();

    handle = createShader(mem);

    var unis = std::vector<tmgl::UniformHandle>();
    var uniformCount = getShaderUniforms(handle);
    unis.resize(uniformCount);
    getShaderUniforms(handle, unis.data(), uniformCount);

    std::vector<string> uniNames;

    texSets.clear();
    for (int i = 0; i < uniformCount; i++)
    {
        tmgl::UniformInfo info = {};

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

            if (info.type == tmgl::UniformType::Sampler)
            {
                texSets.push_back(info.name);
            }

            uniforms.push_back(uniform);
            uniNames.push_back(info.name);
        }
    }

    setName(handle, name.c_str());
}

SubShader::~SubShader()
{
    destroy(handle);
    for (auto uniform : uniforms)
    {
        delete uniform;
    }
}

SubShader* SubShader::CreateSubShader(string name, ShaderType type)
{
    if (IN_MAP(fs::ResourceManager::pInstance->loaded_sub_shaders, name))
    {
        return ResMgr->loaded_sub_shaders[name];
    }

    return new SubShader(name, type);
}

Shader::Shader(ShaderInitInfo info)
{
    program = createProgram(info.vertexProgram->handle, info.fragmentProgram->handle, false);

    subShaders.push_back(info.vertexProgram);
    subShaders.push_back(info.fragmentProgram);
    name = info.name;

    ResMgr->loaded_shaders[info.name] = this;
}

void Shader::Push(int viewId, MaterialOverride* overrides, size_t oc)
{
    std::unordered_map<std::string, MaterialOverride> m_overrides;

    if (overrides != nullptr)
    {
        m_overrides.reserve(oc); // Reserve space to avoid reallocation
        for (size_t i = 0; i < oc; ++i)
        {
            m_overrides.emplace(overrides[i].name, overrides[i]);
        }
    }

    for (auto& shader : subShaders)
    {
        for (auto& uni : shader->uniforms)
        {
            if (overrides != nullptr && m_overrides.contains(uni->name))
            {
                const auto& ovr = m_overrides.at(uni->name);
                uni->v4 = ovr.v4;
                uni->m3 = ovr.m3;
                uni->m4 = ovr.m4;
                uni->tex = ovr.tex;
                uni->forcedSamplerIndex = ovr.forcedSamplerIndex;
            }

            uni->Use(shader);
        }
    }

    submit(viewId, program);
}


Shader::~Shader()
{
    destroy(program);
    for (var shader : subShaders)
    {
        delete shader;
    }
}

void Shader::Reload()
{
    if (isValid(program))
    {
        destroy(program);
    }

    for (auto sub_shader : subShaders)
    {
        sub_shader->Reload();
    }

    program = createProgram(subShaders[0]->handle, subShaders[1]->handle, false);


    std::cout << "Reloaded shader (" << name << ")" << std::endl;
}

Shader* Shader::CreateShader(ShaderInitInfo info)
{
    if (info.name == "UNDEFINED")
    {
        std::hash<string> hsh;
        info.name = std::generateRandomString(10) + std::to_string(
            hsh(info.fragmentProgram->name + info.vertexProgram->name));
    }

    if (IN_MAP(ResMgr->loaded_shaders, info.name))
    {
        return ResMgr->loaded_shaders[info.name];
    }

    return new Shader(info);
}

Shader* Shader::CreateShader(string vertex, string fragment)
{
    var info = ShaderInitInfo{};
    info.vertexProgram = SubShader::CreateSubShader(vertex, SubShader::Vertex);
    info.fragmentProgram = SubShader::CreateSubShader(fragment, SubShader::Fragment);


    if (info.name == "UNDEFINED")
    {
        std::hash<string> hsh;
        info.name = info.vertexProgram->name + "__" + info.fragmentProgram->name;
    }

    if (IN_MAP(ResMgr->loaded_shaders, info.name))
    {
        return ResMgr->loaded_shaders[info.name];
    }

    return new Shader(info);
}

ComputeShader::ComputeShader(SubShader* shader)
{
    program = createProgram(shader->handle, true);
    internalShader = shader;

    ResMgr->loaded_compute_shaders[shader->name + "_CMP"] = this;
}

void ComputeShader::SetUniform(string name, tmgl::UniformType::Enum type, const void* data)
{
    var uni = createUniform(name.c_str(), type);

    setUniform(uni, data);

    destroy(uni);
}

void ComputeShader::SetMat4(string name, glm::mat4 m)
{
    // internalShader->GetUniform()
    for (int i = 0; i < 4; ++i)
    {
        SetUniform(name + "[" + std::to_string(i) + "]", tmgl::UniformType::Vec4, math::vec4toArray(m[0]));
    }
}

void ComputeShader::SetVec4(string name, glm::vec4 v)
{
    SetUniform(name, tmgl::UniformType::Vec4, math::vec4toArray(v));
}

void ComputeShader::Run(int vid, glm::vec3 groups)
{
    for (var uni : internalShader->uniforms)
    {
        uni->Use(internalShader);
    }

    dispatch(program, groups.x, groups.y, groups.z);
}

ComputeShader::~ComputeShader()
{
    destroy(program);
    delete internalShader;
}

ComputeShader* ComputeShader::CreateComputeShader(SubShader* shader)
{
    if (IN_MAP(ResMgr->loaded_compute_shaders, shader->name+"_CMP"))
    {
        return ResMgr->loaded_compute_shaders[shader->name + "_CMP"];
    }

    return new ComputeShader(shader);
}

void MaterialState::SetWrite(u64 flag)
{
    writeR = false;
    writeG = false;
    writeB = false;
    writeA = false;
    writeZ = false;

    if (flag & TMGL_STATE_WRITE_R)
        writeR = true;
    if (flag & TMGL_STATE_WRITE_G)
        writeG = true;
    if (flag & TMGL_STATE_WRITE_B)
        writeB = true;
    if (flag & TMGL_STATE_WRITE_A)
        writeA = true;
    if (flag & TMGL_STATE_WRITE_Z)
        writeZ = true;
}

MaterialOverride* Material::GetUniform(string name, bool force)
{
    for (auto& override : overrides)
    {
        if (override.name == name)
        {
            return &override;
        }
    }

    if (force)
    {
        MaterialOverride ovr;
        ovr.name = name;
        overrides.push_back(ovr);
        return &overrides.back();
    }

    return nullptr;
}


u64 Material::GetMaterialState()
{
    u64 v = state.cull;
    v |= state.depth;

    if (state.writeR)
        v |= TMGL_STATE_WRITE_R;
    if (state.writeG)
        v |= TMGL_STATE_WRITE_G;
    if (state.writeB)
        v |= TMGL_STATE_WRITE_B;
    if (state.writeA)
        v |= TMGL_STATE_WRITE_A;
    if (state.writeZ)
        v |= TMGL_STATE_WRITE_Z;

    v |= TMGL_STATE_BLEND_FUNC(state.srcAlpha, state.dstAlpha);
    v |= TMGL_STATE_BLEND_ALPHA;


    return v;
}

Material::Material(Shader* shader)
{

    Reload(shader);
}

void Material::Reload(Shader* shader)
{
    if (shader)
    {

        std::vector<MaterialOverride> texture_overrides;

        for (auto override : overrides)
        {
            if (override.type == tmgl::UniformType::Sampler)
                texture_overrides.push_back(override);
        }

        overrides.clear();

        for (auto ovr : texture_overrides)
            overrides.push_back(ovr);

        this->shader = shader;
        for (auto sub_shader : shader->subShaders)
        {
            for (auto uniform : sub_shader->uniforms)
            {
                for (auto textureOverride : texture_overrides)
                {
                    if (textureOverride.name == uniform->name)
                        continue;
                }

                var ovr = MaterialOverride();
                ovr.name = uniform->name;
                ovr.shaderType = sub_shader->type;
                ovr.type = uniform->type;

                overrides.push_back(ovr);
            }
        }
    }
    else
    {
        Reload(defaultShader);
    }
}

Material::~Material()
{
    delete[] overrides.data();
}

Mesh::~Mesh()
{
    destroy(ibh);
    destroy(vbh);
    for (auto vertex_buffer : vertexBuffers)
    {
        destroy(vertex_buffer);
    }
    destroy(indexBuffer);

    delete[] vertices;
    delete[] indices;
}

void Mesh::use()
{
    if (origin == mo_loaded)
    {
        setVertexBuffer(0, vbh, 0, vertexCount);
        setIndexBuffer(ibh, 0, indexCount);
    }
    else
    {
        setVertexBuffer(0, vertexBuffers[0], 0, vertexCount);
        setIndexBuffer(indexBuffer, 0, indexCount);
    }
}

void Mesh::draw(glm::mat4 transform, Material* material, glm::vec3 spos, u32 layer, u32 renderLayer,
                std::vector<glm::mat4> anims)
{
    var drawCall = DrawCall();

    drawCall.mesh = this;
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = material->state.matrixMode;
    drawCall.sortedPosition = spos;
    drawCall.layer = math::packU32ToU64(renderLayer, layer);

    //drawCall.transformMatrices = std::vector<MatrixArray>(MAX_BONE_MATRICES + 1);
    drawCall.transformMatrix = transform;

    drawCall.animationMatrices = anims;
    /*
    for (int i = 0; i < anims.size(); ++i)
    {
        var m = anims[i];
        drawCall.transformMatrices[i + 1] = GetMatrixArray(m);
    }
    

    
    for (int i = anims.size(); i < MAX_BONE_MATRICES; ++i)
    {
        drawCall.transformMatrices[i + 1] = GetMatrixArray(glm::mat4(1.0));
    }
    */


    drawCall.matrixCount = anims.size();

    drawCall.program = material->shader;
    if (material->overrides.size() > 0)
    {
        auto _overrides = new MaterialOverride[material->overrides.size()];

        std::copy(material->overrides.begin(), material->overrides.end(), _overrides);

        drawCall.overrides = _overrides;
        drawCall.overrideCt = material->overrides.size();
    }
    else
        drawCall.overrides = nullptr;

    pushDrawCall(drawCall);
}

Texture* Model::GetTextureFromName(string name)
{

    for (Texture* tex : textures)
    {
        if (tex->name == name)
            return tex;
    }

    return nullptr;
}

Material* Model::CreateMaterial(int index, Shader* shader)
{
    return CreateMaterial(materials[index], shader);
}

Material* Model::CreateMaterial(MaterialDescription* materialDesc, Shader* shader)
{
    var material = new Material(shader);

    material->name = materialDesc->Name;
    material->GetUniform("u_color", true)->v4 = Color::White.getData();

    for (auto [uniform, texture] : materialDesc->Textures)
    {
        var t = GetTextureFromName(texture);
        var uni = material->GetUniform(uniform, true);
        uni->type = tmgl::UniformType::Sampler;
        uni->tex = t;
    }

    return material;
}

Animation* Model::GetAnimation(string name)
{
    for (auto animation : animations)
    {
        if (animation->name == name)
            return animation;
    }
    return nullptr;
}

int Model::GetAnimationIndex(string name)
{
    int i = 0;
    for (auto animation : animations)
    {
        if (animation->name == name)
            return i;
        i++;
    }
    return -1;
}

MaterialDescription* Model::GetMaterial(string name)
{
    for (auto material : materials)
    {
        if (material->Name == name)
            return material;
    }

    return nullptr;
}

void SceneDescription::Node::SetParent(Node* parent)
{
    if (this->parent)
    {
        this->parent->children.erase(std::find(this->parent->children.begin(), this->parent->children.end(), this));
        this->parent = nullptr;
    }

    if (parent)
    {

        this->parent = parent;
        this->parent->children.push_back(this);
    }

}

void SceneDescription::Node::AddChild(Node* child)
{
    child->SetParent(this);
}

SceneDescription::Node::Node(fs::BinaryReader* reader, SceneDescription* scene)
{
    name = reader->ReadString();

    position = reader->ReadVec3();

    var x = position.x;
    var z = position.z;
    //position.x = z;
    //position.z = x;

    std::cout << name << ": " << std::to_string(position) << std::endl;

    rotation = reader->ReadQuat();

    scale = reader->ReadVec3();
    this->scene = scene;

    var meshCount = reader->ReadInt32();

    if (meshCount > 0)
    {
        for (int i = 0; i < meshCount; ++i)
        {
            meshIndices.push_back(reader->ReadInt32());
        }
    }

    isBone = reader->ReadInt32() == 0;

    var childCount = reader->ReadInt32();

    for (int i = 0; i < childCount; ++i)
    {
        children.push_back(new Node(reader, scene));
    }
}

SceneDescription::Node::Node(aiNode* node, SceneDescription* scene)
{
    name = node->mName.C_Str();
    glm::quat rot;
    aiVector3D aiPos, aiScale;
    aiQuaternion aiRot;
    node->mTransformation.Decompose(aiScale, aiRot, aiPos);
    position = math::convertVec3(aiPos);
    scale = math::convertVec3(aiScale);
    rot = glm::quat(aiRot.w, aiRot.x, aiRot.y, aiRot.z);
    rotation = rot;
    this->scene = scene;
    for (int i = 0; i < node->mNumMeshes; ++i)
    {
        meshIndices.push_back(node->mMeshes[i]);
    }
    for (int i = 0; i < node->mNumChildren; ++i)
    {
        var c = (new Node(node->mChildren[i], scene));
        c->SetParent(this);
    }


}

SceneDescription::Node* SceneDescription::Node::GetNode(string name, bool isPath)
{
    if (!isPath)
    {

        if (this->name == name)
            return this;
        for (auto child : children)
        {
            var node = child->GetNode(name);
            if (node)
                return node;
        }
        return nullptr;
    }
    // Create a stringstream object to str
    std::stringstream ss(name);

    // Temporary object to store the splitted
    // string
    string t;

    // Delimiter
    char del = '/';

    auto node = this;
    Node* parentNode = nullptr;

    // Splitting the str string by delimiter
    while (getline(ss, t, del))
    {
        if (node)
        {
            parentNode = node;
            node = node->GetNode(t);
            if (!node)
            {
                std::cout << "Null at " << t << std::endl;
                break;
            }
        }
    }

    return node;
}

SceneDescription::Node* SceneDescription::GetNode(string name, bool isPath)
{
    if (isPath)
    {
        // Create a stringstream object to str
        std::stringstream ss(name);

        // Temporary object to store the splitted
        // string
        string t;

        // Delimiter
        char del = '/';

        Node* node = rootNode;
        Node* parentNode = nullptr;

        // Splitting the str string by delimiter
        while (getline(ss, t, del))
        {
            if (node)
            {
                parentNode = node;
                node = node->GetNode(t);
                if (!node)
                {
                    std::cout << "Null at " << t << std::endl;
                    break;
                }
            }
        }

        return node;
    }
    return rootNode->GetNode(name);
}

SceneDescription::Node* SceneDescription::Node::GetNode(aiNode* node)
{
    {
        bool p = true;
        if (parent && node->mParent)
        {
            p = parent->name == node->mParent->mName.C_Str();
        }
        if (p)
        {
            if (name == node->mName.C_Str())
                return this;
        }
    }

    for (auto child : children)
    {
        var n = child->GetNode(node);
        if (n)
            return n;
    }
    return nullptr;
}

Model* SceneDescription::GetModel(string name)
{
    for (auto model : models)
    {
        if (model->name == name)
            return model;
    }

    return nullptr;
}

Mesh* SceneDescription::GetMesh(int idx)
{
    var offset = 0;
    for (Model* model : models)
    {
        if (idx >= model->meshes.size() + offset)
        {
            offset += model->meshes.size();
            continue;
        }

        return model->meshes[idx - offset];
    }

    return nullptr;
}

SceneDescription::Node* SceneDescription::GetNode(aiNode* node)
{
    return rootNode->GetNode(node);
}

SceneDescription::Node* SceneDescription::GetNode(aiMesh* mesh, int index)
{
    var nodes = rootNode->GetAllChildren();
    nodes.push_back(rootNode);

    for (auto value : nodes)
    {
        if (value->name == mesh->mName.C_Str())
        {
            return value;
        }
        for (int meshIndex : value->meshIndices)
        {
            if (meshIndex == index)
            {
                return value;
            }
        }
    }

    return nullptr;

}

std::vector<SceneDescription::Node*> SceneDescription::GetAllChildren()
{
    return rootNode->GetAllChildren();
}

tmt::obj::Object* SceneDescription::Node::ToObject(int modelIndex)
{
    var obj = new obj::Object();

    obj->name = name;
    obj->position = position;
    obj->rotation = (rotation);
    obj->scale = scale;

    for (int mesh_index : meshIndices)
    {
        var mesh = scene->GetMesh(mesh_index);
        var mshObj = obj::MeshObject::FromBasicMesh(mesh);
        mshObj->name = "Mesh";
        mshObj->material = mesh->model->CreateMaterial(mesh->model->materialIndices[mesh_index], defaultShader);
        mshObj->SetParent(obj);
    }

    for (auto child : children)
    {
        bool isArmature = (child->name == "Armature");
        if (isArmature)
        {
            var skele = new SkeletonObject();

            if (modelIndex > -1)
                skele->skeleton = scene->models[modelIndex]->skeleton;

            skele->Load(child);


            obj->AddChild(skele);
            continue;
        }

        int midx = -1;
        if (child->name.starts_with("TMDL_"))
        {
            var model = scene->GetModel(child->name);
            if (model)
            {
                int j = 0;
                for (auto m : scene->models)
                {
                    if (m == model)
                        break;
                    j++;
                }

                midx = j;
            }
        }

        obj->AddChild(child->ToObject(midx));
    }

    return obj;
}

std::vector<SceneDescription::Node*> SceneDescription::Node::GetAllChildren()
{
    var c = std::vector<Node*>();

    for (auto child : children)
    {
        c.push_back(child);
        var c2 = child->GetAllChildren();
        for (auto value : c2)
            c.push_back(value);
    }

    return c;

}

SceneDescription::SceneDescription(string path)
{

    if (!std::filesystem::exists(path))
    {
        std::cout << "Scene does not exist! " << path << std::endl;
    }

    this->path = path;

    if (path.ends_with(".tmdl"))
    {
        var reader = new fs::BinaryReader(path);
        if (!reader->CheckSignature("TSCN"))
        {
            std::cout << "Incorrect scene format!" << std::endl;
            return;
        }

        name = reader->ReadString();
        rootNode = new Node(reader, this);


        var modelCount = reader->ReadInt32();
        for (int i = 0; i < modelCount; ++i)
        {
            models.push_back(new Model(reader, this));
        }

        reader->close();
        delete reader;
    }
    else
    {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                               aiProcess_PopulateArmatureData);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        var fpath = std::filesystem::path(path);

        name = fpath.filename().filename().string();
        rootNode = new Node();
        rootNode->name = name;
        rootNode->scene = this;

        var realRoot = new Node(scene->mRootNode, this);
        realRoot->SetParent(rootNode);

        var c = rootNode->GetAllChildren();

        for (auto value : c)
        {
            if (value->meshIndices.size() > 0)
            {
                //value->SetParent(rootNode);
            }
            if (value->name.starts_with("Armature_"))
            {
                value->name.erase(value->name.find("Armature_"), strlen("Armature_"));
            }
        }

        {
            var model = new Model(scene, this);
            models.push_back(model);
        }

        import.FreeScene();
    }

    ResMgr->loaded_scene_descs[path] = this;
}

tmt::obj::Object* SceneDescription::ToObject()
{
    var root = new obj::Object();
    root->name = name;

    for (auto odel : models)
    {
        var obj = odel->CreateObject();
        obj->SetParent(root);
    }

    return root;
}

SceneDescription::~SceneDescription()
{
    models.clear();
}

SceneDescription* SceneDescription::CreateSceneDescription(string path)
{
    if (ResMgr->loaded_scene_descs.contains(path))
    {
        return ResMgr->loaded_scene_descs[path];
    }

    return new SceneDescription(path);
}

BoneObject::BoneObject(Skeleton::Bone* bone)
{
    this->bone = bone;
    name = bone->name;
    position = bone->position;
    rotation = bone->rotation;
    scale = bone->scale;
}

void BoneObject::Load(SceneDescription::Node* node)
{
    for (auto child : node->children)
    {
        if (child->isBone)
        {
            var bone = new BoneObject();
            bone->name = child->name;
            bone->position = child->position;
            bone->rotation = (child->rotation);
            bone->scale = child->scale;
            bone->Load(child);
            bone->SetParent(this);
        }
    }
}

glm::mat4 BoneObject::GetGlobalOffsetMatrix()
{
    var offset = GetOffsetMatrix();

    if (parent)
    {
        var boneParent = parent->Cast<BoneObject>();

        if (boneParent)
        {
            offset *= boneParent->GetGlobalOffsetMatrix();
        }
    }

    return offset;
}

glm::mat4 Skeleton::Bone::GetTransformation()
{
    if (transformation == glm::mat4(-1))
    {
        //transformation = glm::mat4(1.);

        transformation = glm::mat4(1.0f);

        glm::mat4 rt = toMat4(rotation); // Quaternion to matrix

        transformation[3] = glm::vec4(position, 1.0f); // Translation directly applied
        transformation = transformation * rt; // Apply rotation
        transformation[0] *= scale.x; // Apply scale
        transformation[1] *= scale.y;
        transformation[2] *= scale.z;
    }
    return transformation;
}

glm::mat4 Skeleton::Bone::GetWorldTransform()
{
    var transform = GetTransformation();

    var parent = skeleton->GetParent(this);

    if (parent)
    {
        transform *= parent->GetWorldTransform();
    }
    else
    {

    }

    return transform;
}

void Skeleton::Bone::CopyTransformation(glm::mat4 m)
{
    glm::vec3 pos, scl;
    glm::quat rot;
    glm::vec3 skew;
    glm::vec4 perspective;
    decompose(m, scl, rot, pos, skew, perspective);
    position = pos;
    rotation = rot;
    scale = scl;
}

glm::mat4 BoneObject::GetOffsetMatrix()
{
    glm::mat4 offset(1.0);
    if (bone != nullptr)
    {
        offset = bone->skeleton->boneInfoMap[name].offset;
    }
    return offset;
}

void BoneObject::Update()
{
    if (copyBone)
    {
        position = copyBone->position;
        rotation = copyBone->rotation;
        scale = copyBone->scale;
    }

    //debug::Gizmos::DrawSphere(GetGlobalPosition(), 0.25);

    Object::Update();
}

void SkeletonObject::Load(SceneDescription::Node* node)
{
    name = node->name;
    for (auto child : node->children)
    {
        if (child->isBone)
        {
            var bone = new BoneObject();
            bone->name = child->name;
            bone->position = child->position;
            bone->rotation = (child->rotation);
            bone->scale = child->scale;
            bone->Load(child);
            bone->SetParent(this);
        }
    }

    bones = GetObjectsFromType<BoneObject>();
}

BoneObject* SkeletonObject::GetBone(string name)
{
    for (auto bone : bones)
    {
        if (bone->name == name)
            return bone;
    }

    return nullptr;
}

bool SkeletonObject::IsSkeletonBone(BoneObject* bone)
{
    return std::find(bones.begin(), bones.end(), bone) != bones.end();
}


Animator::Animator()
{

}

glm::mat4 Animator::AnimationBone::Update(float animationTime, Object* obj)
{
    var translation = InterpolatePosition(animationTime);
    var rotation = InterpolateRotation(animationTime);
    var scale = InterpolateScaling(animationTime);
    obj->position = translation;
    obj->rotation = rotation;
    obj->scale = scale;


    return localTransform;
}

int Animator::AnimationBone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < channel->positions.size() - 1; ++index)
    {
        if (animationTime <= channel->positions[index + 1]->time)
            return index;
    }
    assert(0);
}

int Animator::AnimationBone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < channel->rotations.size() - 1; ++index)
    {
        if (animationTime <= channel->rotations[index + 1]->time)
            return index;
    }
    assert(0);
}

int Animator::AnimationBone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < channel->scales.size() - 1; ++index)
    {
        if (animationTime <= channel->scales[index + 1]->time)
            return index;
    }
    assert(0);
}

float Animator::AnimationBone::GetScaleFactor(float lasttime, float nexttime, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lasttime;
    float framesDiff = nexttime - lasttime;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::vec3 Animator::AnimationBone::InterpolatePosition(float animationTime)
{
    if (1 == channel->positions.size())
    {
        var finalPosition = channel->positions[0]->value;
        return finalPosition;
    }

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->positions[p0Index]->time, channel->positions[p1Index]->time, animationTime);
    glm::vec3 finalPosition =
        mix(channel->positions[p0Index]->value, channel->positions[p1Index]->value, scaleFactor);
    return finalPosition;
}

glm::quat Animator::AnimationBone::InterpolateRotation(float animationTime)
{
    if (1 == channel->rotations.size())
    {
        auto finalRotation = normalize(channel->rotations[0]->value);
        return finalRotation;
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->rotations[p0Index]->time, channel->rotations[p1Index]->time, animationTime);
    glm::quat finalRotation =
        glm::slerp(channel->rotations[p0Index]->value, channel->rotations[p1Index]->value, scaleFactor);
    finalRotation = normalize(finalRotation);
    return finalRotation;
}

glm::vec3 Animator::AnimationBone::InterpolateScaling(float animationTime)
{
    if (1 == channel->scales.size())
    {
        var finalScale = channel->scales[0]->value;
        return finalScale;
    }

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->scales[p0Index]->time, channel->scales[p1Index]->time, animationTime);
    glm::vec3 finalScale = mix(channel->scales[p0Index]->value, channel->scales[p1Index]->value, scaleFactor);
    return finalScale;
}

Animator::AnimationBone* Animator::GetBone(string name)
{
    for (auto animation_bone : animationBones)
    {
        if (animation_bone->channel->name == name)
            return animation_bone;
    }

    return nullptr;
}

void Animator::Update()
{
    if (!skeleton)
    {
        skeleton = parent->GetObjectFromType<SkeletonObject>();
    }

    if (currentAnimation && skeleton)
    {
        time += static_cast<float>(currentAnimation->ticksPerSecond) * (deltaTime);
        if (doLoop)
            time = fmod(time, (currentAnimation->duration));
        else if (time >= currentAnimation->duration)
            time = currentAnimation->duration;

        if (currentAnimation->duration <= 0 && doLoop)
            time = 0;

        if (animationBones.size() < currentAnimation->nodeChannels.size())
            LoadAnimationBones();

        for (auto animation_bone : animationBones)
        {
            animation_bone->Update(time, skeleton->bones[animation_bone->boneId]);
        }

        //CalculateBoneTransform(skeleton->skeleton->GetBone(skeleton->skeleton->rootName), glm::mat4(1.0));

        //skeleton->boneMatrices = pushBoneMatrices;
    }


    Object::Update();
}

void Animator::LoadAnimationBones()
{

    if (!skeleton)
    {
        skeleton = parent->GetObjectFromType<SkeletonObject>();
    }

    if (skeleton)
    {

        for (auto node_channel : currentAnimation->nodeChannels)
        {
            var animBone = new AnimationBone;

            animBone->channel = node_channel;
            animBone->animation = currentAnimation;
            animBone->boneId = skeleton->skeleton->boneInfoMap[node_channel->name].id;

            animationBones.push_back(animBone);
        }
    }
}

void Animator::SetAnimation(Animation* animation)
{

    currentAnimation = animation;
    time = 0;
    animationBones.clear();

    LoadAnimationBones();
}

SkeletonObject::SkeletonObject(Skeleton* skl)
{
    skeleton = skl;

    for (auto bone : skl->bones)
    {
        var obj = new BoneObject(bone);

        obj->SetParent(this);

        bones.push_back(obj);
    }

    for (auto bone : skl->bones)
    {
        var obj = GetBone(bone->name);
        for (auto child : bone->children)
        {
            var realChild = GetChild(child);
            if (realChild)
                realChild->SetParent(obj);
        }
    }

    for (auto bone : bones)
    {
        //bone->bone->skeleton->boneInfoMap[bone->name].offset = glm::inverse(bone->GetTransform());
    }
}

void SkeletonObject::Start()
{

    Object::Start();
}

void SkeletonObject::Update()
{

    boneMatrices.clear();
    boneMatrices.resize(bones.size(), glm::mat4(1.0));

    //GetBone(skeleton->rootName)->CalculateBoneMatrix(this, glm::mat4(1.0));
    for (auto bone : bones)
    {
        if (bone->parent == this)
        {
            CalculateBoneTransform(bone->bone, glm::mat4(1.0));
        }
    }

    Object::Update();
}

void SkeletonObject::CalculateBoneTransform(const Skeleton::Bone* skeleBone, glm::mat4 parentTransform)
{
    string nodeName = skeleBone->name;
    glm::mat4 nodeTransform = skeleBone->transformation;
    int idx = 0;

    if (skeleton->boneInfoMap.contains(nodeName))
    {
        idx = skeleton->boneInfoMap[nodeName].id;
    }

    BoneObject* animBone = bones[idx];

    if (animBone)
    {
        nodeTransform = bones[idx]->GetLocalTransform();
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (skeleton->boneInfoMap.contains(nodeName))
    {
        var index = skeleton->boneInfoMap[nodeName].id;
        glm::mat4 offset = skeleton->boneInfoMap[nodeName].offset;
        boneMatrices[index] = globalTransform * (offset);

        /*
        var mat = boneMatrices[index];

        if (time::getTime() == 1)
        {
            if (mat != glm::mat4(1.0))
            {
                std::cout << nodeName << " has an offset matrix issue." << std::endl;

                std::cout << std::endl << "-- Transformation Matrix --" << std::endl;

                for (int x = 0; x < 4; ++x)
                {
                    for (int y = 0; y < 4; ++y)
                    {
                        std::cout << globalTransform[x][y] << " ";
                    }
                    std::cout << std::endl;
                }

                std::cout << std::endl << "-- Offset Matrix --" << std::endl;

                for (int x = 0; x < 4; ++x)
                {
                    for (int y = 0; y < 4; ++y)
                    {
                        std::cout << offset[x][y] << " ";
                    }
                    std::cout << std::endl;
                }

                std::cout << std::endl << "-- Result Matrix (transformation * offset) --" << std::endl;

                for (int x = 0; x < 4; ++x)
                {
                    for (int y = 0; y < 4; ++y)
                    {
                        std::cout << mat[x][y] << " ";
                    }
                    std::cout << std::endl;
                }

                std::cout << std::endl;

                //boneMatrices[index] = glm::inverse(boneMatrices[index]);
            }
        }
        */
    }

    for (string child : skeleBone->children)
    {
        CalculateBoneTransform(skeleton->GetBone(child), globalTransform);
    }
}


Model::Model(string path)
{
    if (path.ends_with(".tmdl"))
    {


        var reader = new fs::BinaryReader(path);


        reader->close();

    }
    else
    {


        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenUVCoords);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        LoadFromAiScene(scene);

        import.FreeScene();
    }
}

Skeleton::Bone* Skeleton::GetBone(string name)
{
    /*
    for (auto value : bones)
    {
        if (value->name == name)
            return value;
    }
    */
    if (boneInfoMap.contains(name) && bones.size() > 0)
    {
        var id = boneInfoMap[name].id;

        if (bones.size() > id)
            return bones[id];
    }

    return nullptr;
}

Skeleton::Bone* Skeleton::GetParent(Bone* b)
{
    for (auto value : bones)
    {
        if (value->name != b->name)
        {
            for (auto child : value->children)
            {
                if (child == b->name)
                    return value;
            }
        }
    }
    return nullptr;
}

Model::Model(const aiScene* scene)
{
    LoadFromAiScene(scene);
}

Model::Model(const aiScene* scene, SceneDescription* description)
{
    LoadFromAiScene(scene, description);
}

void Model::LoadFromAiScene(const aiScene* scene, SceneDescription* description)
{
    name = "TMDL_" + std::string(scene->mName.C_Str());
    skeleton = new Skeleton();

    skeleton->inverseTransform = math::convertMat4(scene->mRootNode->mTransformation.Inverse());


    int maxBones = INT_MAX;
    SceneDescription::Node* modelNode;
    if (description)
    {

        modelNode = new SceneDescription::Node;
        modelNode->name = name;
        modelNode->scene = description;
        modelNode->SetParent(description->rootNode);

    }

    for (int i = 0; i < scene->mNumMeshes; ++i)
    {
        var msh = scene->mMeshes[i];

        std::vector<Vertex> vertices;

        for (int j = 0; j < msh->mNumVertices; ++j)
        {
            aiVector3D pos(0), norm(0), uv(0);
            pos = msh->mVertices[j];
            if (msh->HasNormals())
                norm = msh->mNormals[j];
            if (msh->HasTextureCoords(0))
                uv = msh->mTextureCoords[0][j];

            var vertex = Vertex{};

            vertex.position = glm::vec3{pos.x, pos.y, pos.z};
            vertex.normal = glm::vec3{norm.x, norm.y, norm.z};
            vertex.uv0 = glm::vec2{uv.x, uv.y};
            vertices.push_back(vertex);
        }

        std::vector<u16> indices;

        for (unsigned int j = 0; j < msh->mNumFaces; j++)
        {
            aiFace face = msh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
                indices.push_back(face.mIndices[k]);
        }

        for (int j = 0; j < msh->mNumBones; ++j)
        {
            var b = msh->mBones[j];

            Skeleton::Bone* bone = nullptr;

            int boneId = -1;
            string boneName = b->mName.C_Str();

            if (boneName.starts_with("Armature_"))
            {
                boneName.erase(boneName.find("Armature_"), strlen("Armature_"));
            }

            if (!skeleton->boneInfoMap.contains(boneName))
            {
                BoneInfo boneInfo;
                boneInfo.id = skeleton->boneInfoMap.size();
                boneInfo.offset = math::convertMat4(b->mOffsetMatrix);

                skeleton->boneInfoMap[boneName] = boneInfo;
                boneId = boneInfo.id;
            }
            else
            {
                boneId = skeleton->boneInfoMap[boneName].id;

                skeleton->boneInfoMap[boneName].offset = math::convertMat4(b->mOffsetMatrix);
            }

            if (skeleton->GetBone(boneName) == nullptr)
            {

                bone = new Skeleton::Bone;
                bone->name = boneName;
                bone->skeleton = skeleton;

                skeleton->bones.push_back(bone);
            }
            else
            {
                bone = skeleton->GetBone(boneName);
            }

            for (int k = 0; k < b->mNumWeights; ++k)
            {
                var weight = b->mWeights[k];

                bone->weights.push_back(Skeleton::Bone::VertexWeight{weight.mVertexId, weight.mWeight});
                vertices[weight.mVertexId].SetBoneData(boneId, weight.mWeight);
            }

            if (boneId >= maxBones)
            {
                std::cout << "Null bone data found: " << boneName << std::endl;
            }

            if (description)
            {

                if (j == 0)
                {
                    var anode = description->GetNode("Armature");

                    skeleton->rootName = boneName;
                    if (anode != nullptr && anode != description->rootNode)
                    {
                        anode->SetParent(modelNode);
                    }
                }

                var bnode = description->GetNode(boneName);
                if (bnode)
                    bnode->isBone = true;
            }
        }

        auto verts = new Vertex[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), verts);

        auto incs = new u16[indices.size()];
        std::copy(indices.begin(), indices.end(), incs);

        var mesh = createMesh(verts, incs, vertices.size(), indices.size(), Vertex::getVertexLayout(), this,
                              msh->mName.C_Str());

        meshes.push_back(mesh);
        materialIndices.push_back(msh->mMaterialIndex);

        for (int j = 0; j < msh->mNumBones; ++j)
        {
            var bone = msh->mBones[j];

            mesh->bones.push_back(bone->mName.C_Str());
        }


        if (description)
        {
            var mnd = description->GetNode(msh, i);

            if (mnd)
            {

                var meshNode = new SceneDescription::Node;
                meshNode->name = "TMSH_" + mnd->name;

                meshNode->meshIndices.push_back(i);
                meshNode->SetParent(modelNode);

                meshNode->position = mnd->position;
                meshNode->rotation = mnd->rotation;
                meshNode->scale = mnd->scale;
                meshNode->scene = description;

                mnd->SetParent(nullptr);

                delete mnd;
            }
        }

    }

    /*
    if (description)
    {

        var c = description->GetAllChildren();

        for (auto value : c)
        {
            if (value->meshIndices.size() > 0 && !value->name.starts_with("TMSH_"))
            {
                value->SetParent(nullptr);
                delete value;
            }
        }
    }
    */

    for (int i = 0; i < scene->mNumMaterials; ++i)
    {
        var mat = scene->mMaterials[i];

        var desc = new MaterialDescription;

        desc->Name = mat->GetName().C_Str();

        for (int i = 0; i < mat->mNumProperties; ++i)
        {
            var property = mat->mProperties[i];
        }

        std::map<int, string> typeToName = {
            {aiTextureType_DIFFUSE, "s_texColor"}
        };

        for (int i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
        {
            var type = static_cast<aiTextureType>(i);
            var textureCount = mat->GetTextureCount(type);

            if (textureCount > 0)
            {

                var typeName = std::to_string(type);

                if (typeToName.contains(type))
                {
                    typeName = typeToName.at(type);
                }

                for (int j = 0; j < textureCount; ++j)
                {
                    aiString path;
                    aiTextureMapping mapping;
                    unsigned int uvindex;
                    float blend;
                    aiTextureOp op;
                    aiTextureMapMode mapmode[3];

                    aiReturn result = mat->GetTexture(type, j, &path, &mapping, &uvindex, &blend, &op, mapmode);

                    u64 flags = 0;

                    switch (mapmode[0])
                    {
                        case aiTextureMapMode_Clamp:
                            flags |= TMGL_SAMPLER_U_CLAMP;
                            break;
                        case aiTextureMapMode_Mirror:
                            flags |= TMGL_SAMPLER_U_MIRROR;
                            break;
                        case aiTextureMapMode_Wrap:
                            break;
                    }

                    switch (mapmode[1])
                    {
                        case aiTextureMapMode_Clamp:
                            flags |= TMGL_SAMPLER_V_CLAMP;
                            break;
                        case aiTextureMapMode_Mirror:
                            flags |= TMGL_SAMPLER_V_MIRROR;
                            break;
                        case aiTextureMapMode_Wrap:
                            break;
                    }

                    if (result == aiReturn_SUCCESS)
                    {
                        var _tex = scene->GetEmbeddedTexture(path.C_Str());


                        Texture* tex = nullptr;

                        if (_tex)
                        {
                            tex = GetTextureFromName(_tex->mFilename.C_Str());

                            if (!tex)
                            {
                                tex = new Texture(_tex->pcData, _tex->mWidth, flags);

                                if (isValid(tex->handle))
                                {
                                    tex->name = _tex->mFilename.C_Str();

                                    textures.push_back(tex);
                                }
                                else
                                {
                                    tex = nullptr;
                                }
                            }
                        }
                        else
                        {
                            if (description)
                            {
                                var fpath = std::filesystem::path(description->path).parent_path();

                                string _path = path.C_Str();

                                if (type == aiTextureType_DIFFUSE && _path.ends_with("_NRM.png"))
                                {
                                    _path.erase(_path.find("_NRM"), 4);
                                }

                                var file = std::filesystem::path(_path);


                                tex = Texture::CreateTexture(fpath.string() + "/" + file.string(), flags);

                                if (tex)
                                {


                                    if (isValid(tex->handle))
                                    {

                                        textures.push_back(tex);
                                    }
                                }

                            }


                        }

                        if (tex)
                            desc->Textures.insert(std::make_pair(typeName, tex->name));
                    }
                }
            }
        }


        materials.push_back(desc);
    }

    for (int i = 0; i < scene->mNumSkeletons; ++i)
    {
        var skl = scene->mSkeletons[i];

        break;
    }

    if (scene->mSkeletons == nullptr)
    {
        for (auto value : skeleton->bones)
        {
            var node = scene->mRootNode->FindNode(value->name.c_str());

            value->CopyTransformation(math::convertMat4(node->mTransformation));

            for (int i = 0; i < node->mNumChildren; ++i)
            {
                var child = node->mChildren[i];
                value->children.push_back(child->mName.C_Str());
            }
        }
    }

    for (int i = 0; i < scene->mNumAnimations; ++i)
    {
        var anim = scene->mAnimations[i];

        var animation = new Animation();
        animation->LoadFromAiAnimation(anim);


        animations.push_back(animation);

        //break;
    }

}


Model::~Model()
{
    delete[] meshes.data();
    delete[] textures.data();
    delete[] materials.data();
    delete[] animations.data();

    delete skeleton;
}

glm::vec3 SceneDescription::Node::GetGlobalPosition()
{
    var p = position;

    if (parent)
        p += parent->GetGlobalPosition();

    return p;
}

Model::Model(fs::BinaryReader* reader, SceneDescription* description)
{
    var tmdlSig = reader->ReadString(4);

    var scale = reader->ReadSingle();

    if (tmdlSig != "TMDL")
    {
        std::cout << "Incorrect tmdl format!" << std::endl;
        return;
    }

    name = reader->ReadString();

    SceneDescription::Node* modelNode;
    if (description)
    {

        modelNode = new SceneDescription::Node;
        modelNode->name = name;
        modelNode->scene = description;
        modelNode->SetParent(description->rootNode);
    }

    var meshCount = reader->ReadInt32();

    for (int i = 0; i < meshCount; ++i)
    {
        var tmshSig = reader->ReadString(4);
        if (tmshSig == "TMSH")
        {
            var name = reader->ReadString();

            var vtxCount = reader->ReadInt32();
            var vertices = new Vertex[vtxCount];

            reader->Skip(4);

            for (int i = 0; i < vtxCount; ++i)
            {
                glm::vec3 p = math::convertVec3(reader->ReadFloatArray(3)) * scale;
                glm::vec3 n = math::convertVec3(reader->ReadFloatArray(3));
                glm::vec2 u = math::convertVec2(reader->ReadFloatArray(2));

                var boneIdxCount = reader->ReadInt32();
                var boneIds = reader->ReadArray<s32>(boneIdxCount);

                var weightCount = reader->ReadInt32();
                var weights = reader->ReadArray<float>(weightCount);

                Vertex vtx = {p, n, u, math::convertIVec4(boneIds, boneIdxCount),
                              math::convertVec4(weights, weightCount)};
                vertices[i] = vtx;
            }

            reader->Skip(4);
            var idxCount = reader->ReadInt32();
            var incs = new u16[idxCount];

            for (int i = 0; i < idxCount; ++i)
            {
                incs[i] = reader->ReadUInt16();
            }

            if (vtxCount == 2162)
            {
                var vert = vertices[116];
                std::cout << vert.position.x << std::endl;
            }

            var materialIndex = reader->ReadInt32();

            meshes.push_back(createMesh(vertices, incs, vtxCount, idxCount, Vertex::getVertexLayout(), this));
            materialIndices.push_back((materialIndex));
        }
    }

    var textureCount = reader->ReadInt32();

    for (int i = 0; i < textureCount; ++i)
    {
        long p = reader->tellg();
        var texSig = reader->ReadString(4);
        if (texSig == "TTEX")
        {

            var name = reader->ReadString();

            var width = reader->ReadInt32();
            var height = reader->ReadInt32();
            var channelCount = reader->ReadInt32();

            var size = width * height * channelCount;
            std::vector<unsigned char> dataV;
            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    for (int j = 0; j < channelCount; ++j)
                    {
                        dataV.push_back(reader->ReadByte());
                    }
                }
            }

            var format = tmgl::TextureFormat::RGBA8;

            switch (channelCount)
            {
                case 1:
                    format = tmgl::TextureFormat::R8;
                    break;
                case 2:
                    format = tmgl::TextureFormat::RG8;
                    break;
                case 3:
                    format = tmgl::TextureFormat::RGB8;
                    break;
                default:
                    break;
            }

            var tex = new Texture(width, height, format, TMGL_SAMPLER_UVW_CLAMP, tmgl::copy(dataV.data(), size), name);


            textures.push_back(tex);
        }
    }


    var materialCount = reader->ReadInt32();
    for (int i = 0; i < materialCount; ++i)
    {
        reader->Skip(4);
        var materialDescription = new MaterialDescription();
        var name = reader->ReadString();

        materialDescription->Name = name;

        var texCount = reader->ReadInt32();
        for (int i = 0; i < texCount; ++i)
        {
            var uniform = reader->ReadString();
            var texture = reader->ReadString();

            materialDescription->Textures.insert(std::make_pair(uniform, texture));
        }
        materials.push_back(materialDescription);
    }

    skeleton = new Skeleton(reader);

    var animCount = reader->ReadInt32();

    int preAnim = reader->tellg();
    int postAnim = -1;

    int animSize = -1;

    for (int i = 0; i < animCount; ++i)
    {
        if (animSize == -1)
            animations.push_back(new Animation(reader));

        if (i == 0 && postAnim == -1)
        {
            postAnim = reader->tellg();
            //animSize = postAnim - preAnim;
        }

        if (animSize != -1)
        {
            reader->Skip(animSize);
        }
    }
}

Animation::Animation(fs::BinaryReader* reader)
{
    var sig = reader->ReadString(4);

    name = reader->ReadString();
    duration = reader->ReadSingle();
    ticksPerSecond = reader->ReadInt32();

    var channelCount = reader->ReadInt32();
    for (int i = 0; i < channelCount; ++i)
    {
        var nodeChannel = new NodeChannel;

        var nodeName = reader->ReadString();
        var id = reader->ReadInt32();

        nodeChannel->name = nodeName;
        nodeChannel->id = id;

        var pct = reader->ReadInt32();
        for (int j = 0; j < pct; ++j)
        {
            var key = new NodeChannel::NodeKey<glm::vec3>();
            key->time = reader->ReadSingle();
            key->value = reader->ReadVec3();
            nodeChannel->positions.push_back(key);
        }

        var rct = reader->ReadInt32();
        for (int j = 0; j < rct; ++j)
        {
            var key = new NodeChannel::NodeKey<glm::quat>();
            key->time = reader->ReadSingle();
            key->value = reader->ReadQuat();
            nodeChannel->rotations.push_back(key);
        }

        var sct = reader->ReadInt32();
        for (int j = 0; j < sct; ++j)
        {
            var key = new NodeChannel::NodeKey<glm::vec3>();
            key->time = reader->ReadSingle();
            key->value = reader->ReadVec3();
            nodeChannel->scales.push_back(key);
        }


        nodeChannels.push_back(nodeChannel);
    }
}

std::vector<Animation*> Animation::LoadAnimations(string path)
{
    std::vector<Animation*> anims;

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate);

    if (!scene || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return anims;
    }

    if (scene->HasAnimations())
    {
        for (int i = 0; i < scene->mNumAnimations; ++i)
        {
            var a = scene->mAnimations[i];

            var animation = new Animation();

            animation->LoadFromAiAnimation(a);

            anims.push_back(animation);
        }
    }

    return anims;
}


void Animation::LoadFromAiAnimation(aiAnimation* anim)
{
    name = anim->mName.C_Str();
    duration = static_cast<float>(anim->mDuration);
    ticksPerSecond = static_cast<int>(anim->mTicksPerSecond);

    for (int i = 0; i < anim->mNumChannels; ++i)
    {
        var channel = anim->mChannels[i];
        var nodeChannel = new NodeChannel;
        nodeChannel->name = channel->mNodeName.C_Str();

        for (int i = 0; i < channel->mNumPositionKeys; ++i)
        {
            var posKey = channel->mPositionKeys[i];
            nodeChannel->positions.push_back(new NodeChannel::NodeKey<glm::vec3>(
                static_cast<float>(posKey.mTime), math::convertVec3(posKey.mValue)));
        }

        for (int i = 0; i < channel->mNumScalingKeys; ++i)
        {
            var posKey = channel->mScalingKeys[i];
            nodeChannel->scales.push_back(new NodeChannel::NodeKey<glm::vec3>(
                static_cast<float>(posKey.mTime), math::convertVec3(posKey.mValue)));
        }

        for (int i = 0; i < channel->mNumRotationKeys; ++i)
        {
            var posKey = channel->mRotationKeys[i];
            nodeChannel->rotations.push_back(new NodeChannel::NodeKey<glm::quat>(
                static_cast<float>(posKey.mTime), math::convertQuat(posKey.mValue)));
        }

        nodeChannels.push_back(nodeChannel);
    }
}

Skeleton::Skeleton(fs::BinaryReader* reader)
{
    reader->Skip(4);
    var boneCount = reader->ReadInt32();

    for (int i = 0; i < boneCount; ++i)
    {
        var name = reader->ReadString();
        var id = reader->ReadInt32();

        var pos = reader->ReadVec3();
        var rot = reader->ReadQuat();
        var scl = reader->ReadVec3();

        var bone = new Bone();
        bone->name = name;
        bone->position = pos;

        //bone->position.x = pos.z;
        //bone->position.z = pos.x;

        bone->rotation = rot;
        bone->scale = scl;
        bone->skeleton = this;

        glm::mat4 offsetMatrix;

        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                offsetMatrix[y][x] = reader->ReadSingle();
            }
        }

        int childCount = reader->ReadInt32();
        for (int i = 0; i < childCount; ++i)
        {
            var cname = reader->ReadString();
            bone->children.push_back(cname);
        }

        bones.push_back(bone);
        boneInfoMap[name] = {id, offsetMatrix};
    }

    rootName = reader->ReadString();

    for (auto& value : bones)
    {
        var world = value->GetWorldTransform();
        var inverse = glm::inverse(world);

        var diff = inverse - world;
        var diff2 = world + inverse;

        boneInfoMap[value->name].offset = (inverse);

    }
}

tmt::obj::Object* Model::CreateObject(Shader* shdr)
{

    var shader = shdr;

    if (shader == nullptr)
    {
        shader = defaultShader;
    }

    var mdlObj = new obj::Object();

    std::vector<Material*> mats;
    mdlObj->name = name;

    for (auto materialDesc : this->materials)
    {
        mats.push_back(CreateMaterial(materialDesc, shader));
    }


    var midx = 0;
    for (auto value : meshes)
    {
        var mshObj = obj::MeshObject::FromBasicMesh(value);
        mshObj->material = mats[materialIndices[midx]];
        mshObj->SetParent(mdlObj);

        midx++;
    }

    var sklObj = new SkeletonObject(skeleton);
    sklObj->SetParent(mdlObj);

    var animator = new Animator();
    animator->SetParent(mdlObj);

    sklObj->animator = animator;

    return mdlObj;
}

Texture::Texture(aiTexel* texels, int width, u64 flags)
{

    var caps = tmgl::getCaps();
    //stbi_set_flip_vertically_on_load(true);


    //bool f = true;

    int channels;
    u8* data = stbi_load_from_memory((unsigned char*)texels, width, &this->width, &this->height, &channels, 4);

    bool f = this->width < caps->limits.maxTextureSize && this->height < caps->limits.maxTextureSize;


    if (f)
    {


        tmgl::TextureFormat::Enum textureFormat = tmgl::TextureFormat::RGBA8;
        uint64_t textureFlags = flags; // Adjust as needed

        // Create the texture in bgfx, passing the image data directly
        handle = createTexture2D(static_cast<u16>(this->width), static_cast<u16>(this->height), false, 1, textureFormat,
                                 textureFlags, tmgl::copy(data, this->width * this->height * channels));
        format = textureFormat;
    }
    else
    {
        handle.idx = -1;
    }

    stbi_image_free(data);
}

Texture::Texture(string path, bool isCubemap, u64 flags)
{

    uint64_t textureFlags = flags;
    // Adjust as needed
    if (!isCubemap)
    {
        int nrChannels;
        u8* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        var dataSize = width * height * nrChannels;
        auto rgbaData = new unsigned char[dataSize];

        if (nrChannels == 3)
        {
            delete[] rgbaData;
            dataSize = width * height * 4;
            rgbaData = new unsigned char[dataSize];
            for (int i = 0; i < width * height; ++i)
            {
                rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
                rgbaData[i * 4 + 3] = 255; // Add alpha channel with value 1.0
            }
            nrChannels = 4;
        }
        else if (nrChannels == 4)
        {
            for (int i = 0; i < width * height; ++i)
            {
                rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
                rgbaData[i * 4 + 3] = data[i * nrChannels + 3]; // Add alpha channel with value 1.0
            }
            rgbaData = data;
        }
        else if (nrChannels == 2)
        {
            delete[] rgbaData;
            dataSize = width * height * 4;
            rgbaData = new unsigned char[dataSize];
            for (int i = 0; i < width * height; ++i)
            {
                rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                rgbaData[i * 4 + 2] = 0;
                rgbaData[i * 4 + 3] = data[i * nrChannels + 1];
            }
        }


        tmgl::TextureFormat::Enum textureFormat = tmgl::TextureFormat::RGBA8;

        var s = sizeof(rgbaData);

        // Create the texture in bgfx, passing the image data directly
        handle = createTexture2D(static_cast<u16>(width), static_cast<u16>(height), false, 1, textureFormat,
                                 textureFlags, tmgl::copy(rgbaData, dataSize));
        format = textureFormat;

        var fpath = std::filesystem::path(path);
        name = fpath.stem().string();

        stbi_image_free(data);
    }
    else
    {

    }

    fs::ResourceManager::pInstance->loaded_textures.insert(std::make_pair(name, this));

}

Texture::Texture()
{
}

TextureAtlas* TextureAtlas::CreateTexture(string dirPath) { return new TextureAtlas(dirPath); }

TextureAtlas* TextureAtlas::CreateTexture(std::vector<string> paths) { return new TextureAtlas(paths); }

TextureAtlas::~TextureAtlas()
{
}

TextureAtlas::TextureAtlas(string dirPath) :
    Texture()
{

    std::vector<u8> pixels;
    u16 imageCount = 0;

    for (const auto& entry : std::filesystem::directory_iterator(dirPath))
    {
        if (entry.is_regular_file())
        {
            string path = entry.path().generic_string();

            int nrChannels;
            u8* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

            var dataSize = width * height * nrChannels;
            auto rgbaData = new unsigned char[dataSize];

            if (nrChannels == 3)
            {
                delete[] rgbaData;
                dataSize = width * height * 4;
                rgbaData = new unsigned char[dataSize];
                for (int i = 0; i < width * height; ++i)
                {
                    rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                    rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                    rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
                    rgbaData[i * 4 + 3] = 1.0f; // Add alpha channel with value 1.0
                }
                nrChannels = 4;
            }
            else if (nrChannels == 4)
            {
                rgbaData = data;
            }

            for (int i = 0; i < dataSize; ++i)
            {
                pixels.push_back(rgbaData[i]);
            }

            imageCount++;
        }
    }

    tmgl::TextureFormat::Enum textureFormat = tmgl::TextureFormat::RGBA8;
    uint64_t textureFlags = TMGL_SAMPLER_U_MIRROR | TMGL_SAMPLER_V_MIRROR | TMGL_SAMPLER_POINT; // Adjust as needed

    // Create the texture in bgfx, passing the image data directly
    handle = createTexture2D(static_cast<u16>(width), static_cast<u16>(height), false, imageCount, textureFormat,
                             textureFlags,
                             tmgl::copy(pixels.data(),
                                        (static_cast<u32>(pixels.size()) * static_cast<u32>(sizeof(u8)))));
    format = textureFormat;
    setName(handle, "atlas");
}

TextureAtlas::TextureAtlas(std::vector<string> paths)
{
    std::vector<u8> pixels;
    u16 imageCount = 0;

    for (auto path : paths)
    {
        int nrChannels;
        stbi_set_flip_vertically_on_load(true);
        u8* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        var dataSize = width * height * nrChannels;
        auto rgbaData = new unsigned char[dataSize];

        if (nrChannels == 3)
        {
            delete[] rgbaData;
            dataSize = width * height * 4;
            rgbaData = new unsigned char[dataSize];
            for (int i = 0; i < width * height; ++i)
            {
                rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
                rgbaData[i * 4 + 3] = 1.0f; // Add alpha channel with value 1.0
            }
            nrChannels = 4;
        }
        else if (nrChannels == 4)
        {
            rgbaData = data;
        }

        for (int i = 0; i < dataSize; ++i)
        {
            pixels.push_back(rgbaData[i]);
        }

        imageCount++;
    }

    tmgl::TextureFormat::Enum textureFormat = tmgl::TextureFormat::RGBA8;
    uint64_t textureFlags = TMGL_SAMPLER_U_MIRROR | TMGL_SAMPLER_V_MIRROR | TMGL_SAMPLER_POINT; // Adjust as needed

    // Create the texture in bgfx, passing the image data directly
    handle = createTexture2D(
        static_cast<u16>(width), static_cast<u16>(height), false, imageCount, textureFormat, textureFlags,
        tmgl::copy(pixels.data(), (static_cast<u32>(pixels.size()) * static_cast<u32>(sizeof(u8)))));
    format = textureFormat;
    setName(handle, "atlas");
}

Texture* Texture::CreateTexture(string path, bool isCubemap, u64 flags)
{
    if (IN_MAP(ResMgr->loaded_textures, path))
    {
        return ResMgr->loaded_textures[path];
    }

    if (!std::filesystem::exists(path))
        return nullptr;

    return new Texture(path, isCubemap, flags);
}

Texture::Texture(int width, int height, tmgl::TextureFormat::Enum tf, u64 flags, const tmgl::Memory* mem, string name)
{
    handle = createTexture2D(width, height, false, 1, tf, flags, mem);

    setName(handle, name.c_str());

    format = tf;
    this->width = width;
    this->height = height;
    this->name = name;

    ResMgr->loaded_textures[name] = this;
}

Texture::Texture(tmgl::TextureHandle handle)
{
    this->handle = handle;

}

Texture::~Texture()
{
    destroy(handle);
}

RenderTexture::RenderTexture(u16 width, u16 height, tmgl::TextureFormat::Enum format, u16 cf)
{
    const tmgl::Memory* mem = nullptr;
    if (format == tmgl::TextureFormat::RGBA8)
    {
        // std::vector<GLubyte> pixels(width * height * 4, (GLubyte)0xffffffff);
        // mem = tmgl::copy(pixels.data(), width * height* 4);
    }

    u64 textureFlags = TMGL_TEXTURE_RT_WRITE_ONLY;

    bgfx::TextureFormat::Enum depthFormat = isTextureValid(0, false, 1, bgfx::TextureFormat::D16, textureFlags)
        ? bgfx::TextureFormat::D16
        : isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags)
        ? bgfx::TextureFormat::D24S8
        : bgfx::TextureFormat::D32;

    realTexture = new Texture(width, height, format,
                              textureFlags
                              , mem);


    handle = createFrameBuffer(1, &realTexture->handle, true);
    this->format = format;

    //tmgl::setViewName(vid, "RenderTexture");
    //tmgl::setViewClear(vid, cf);
    //tmgl::setViewRect(vid, 0, 0, width, height);
    //setViewFrameBuffer(vid, handle);

    viewId = renderer->viewCache.size();
    setViewFrameBuffer(viewId, handle);

    renderer->viewCache.push_back(this);
}

RenderTexture::RenderTexture()
{
    viewId = renderer->viewCache.size();
    if (viewId == 0)
    {
        tmgl::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);
    }
    else
    {

        var width = renderer->windowWidth;
        var height = renderer->windowHeight;


        depthFormat = tmgl::TextureFormat::D24S8;
        format = tmgl::TextureFormat::RGBA8;

        resize(width, height);
    }
    tmgl::setViewClear(viewId, TMGL_CLEAR_COLOR | TMGL_CLEAR_DEPTH, Color(0.2f, 0.3f, 0.3f, 1).getHex(), 1.0f, 0);

    if (!name.empty())
    {
        tmgl::setViewName(viewId, name.c_str());
    }


    renderer->viewCache.push_back(this);
}


RenderTexture::~RenderTexture()
{
    bgfx::resetView(viewId);
    renderer->viewCache.erase(VEC_FIND(renderer->viewCache, this));
}

void RenderTexture::resize(int width, int height)
{
    if (realTexture)
        delete realTexture;
    if (depthTexture)
        delete depthTexture;
    if (isValid(handle))
    {
        destroy(handle);
    }

    u64 textureFlags = TMGL_TEXTURE_RT;


    realTexture = new Texture(width, height, format, textureFlags);
    depthTexture = new Texture(width, height, depthFormat, textureFlags);

    tmgl::TextureHandle handles[] = {realTexture->handle, depthTexture->handle};

    handle = createFrameBuffer(2, handles, true);

    setViewFrameBuffer(viewId, handle);
}

Font* Font::Create(string path)
{
    if (IN_MAP(ResMgr->loaded_fonts, path))
    {
        return ResMgr->loaded_fonts[path];
    }

    return new Font(path);
}

float Font::CalculateTextSize(string text, float fontSize, float forcedSpacing)
{
    float size = 0;

    if (forcedSpacing == FLT_MAX)
        forcedSpacing = spacing;

    var scl = 1.0f;

    scl = (fontSize / 48);

    for (char value : text)
    {

        var c = characters[value];

        float xPos = size - c.bearing.x * scl;

        size -= (c.advance >> 6) * scl;
    }

    size = -size;

    return size;
}


Font::Font(string path)
{


    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
    {
        std::cout << "Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    tmgl::VertexLayout layout;
    layout.begin();
    layout.add(tmgl::Attrib::Position, 4, tmgl::AttribType::Float);
    layout.end();


    std::vector<u16> indices = {0, 1, 2, 1, 3, 2};

    ibh = createIndexBuffer(tmgl::copy(indices.data(), indices.size() * sizeof(u16)));

    FT_GlyphSlot slot = face->glyph; // <-- This is new

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        }

        // Render as SDF using FreeType's built-in support
        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF))
        {
            std::cerr << "Failed to render SDF glyph!" << std::endl;
        }

        tmgl::TextureFormat::Enum textureFormat = tmgl::TextureFormat::R8;
        uint64_t textureFlags = 0; // Adjust as needed

        var width = face->glyph->bitmap.width;
        var height = face->glyph->bitmap.rows;

        if (width == 0 || height == 0)
        {
            Character character = {glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                   face->glyph->advance.x, nullptr,TMGL_INVALID_HANDLE};
            characters.insert(std::pair<char, Character>(c, character));
            continue;
        }


        var size = width * height;

        // Create the texture in bgfx, passing the image data directly
        var handle = createTexture2D(
            static_cast<u16>(width), static_cast<u16>(height), false, 1,
            textureFormat, textureFlags,
            tmgl::copy(face->glyph->bitmap.buffer, size));

        var tex = new Texture(handle);

        string handleName = "";
        handleName += static_cast<char>(c);
        handleName += "_";
        handleName += face->family_name;

        setName(handle, handleName.c_str());

        tex->name = handleName;

        std::vector<glm::vec4> vertices;

        var v_width = static_cast<float>(width);
        var v_height = static_cast<float>(height);

        vertices.emplace_back(0, 0, v_width, v_height);
        vertices.emplace_back(v_width, 0, v_width, v_height);
        vertices.emplace_back(0, v_height, v_width, v_height);
        vertices.emplace_back(v_width, v_height, v_width, v_height);

        var vbh = createVertexBuffer(tmgl::copy(vertices.data(), (vertices.size() * sizeof(glm::vec4))), layout);

        Character character = {glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(glm::abs(face->glyph->bitmap_left), face->glyph->bitmap_top),
                               face->glyph->advance.x,
                               tex, vbh};
        characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    ResMgr->loaded_fonts[path] = this;
}

float* Camera::GetView()
{
    var Front = GetFront();
    var Up = GetUp();

    float view[16];

    mtxLookAt(view, bx::Vec3(position.x, position.y, position.z), math::convertVec3(position + Front),
              bx::Vec3(Up.x, Up.y, Up.z));

    return view;
}

const float* Camera::GetProjection()
{
    return value_ptr(GetProjection_m4());
}

glm::mat4 Camera::GetView_m4()
{
    //if (mode == Perspective)
    return lookAt(position, position + GetFront(), GetUp());

    return glm::mat4(1.0);
}

glm::mat4 Camera::GetProjection_m4()
{
    float width = renderer->windowWidth;
    float height = renderer->windowHeight;

    if (renderTexture->realTexture)
    {
        width = renderTexture->realTexture->width;
        height = renderTexture->realTexture->height;
    }

    if (width == 0 || height == 0)
    {
        return glm::mat4(1.0);
    }

    if (mode == Perspective)
        return glm::perspective(glm::radians(FOV),
                                width / height,
                                NearPlane, FarPlane);

    return GetOrthoProjection_m4();

}

glm::mat4 Camera::GetOrthoProjection_m4()
{
    float width = renderer->windowWidth;
    float height = renderer->windowHeight;

    if (renderTexture->realTexture)
    {
        width = renderTexture->realTexture->width;
        height = renderTexture->realTexture->height;
    }

    if (width == 0 || height == 0)
    {
        return glm::mat4(1.0);
    }

    float rad = glm::radians(FOV);

    if (!application->is2D)
        rad = 1;

    float left = (width / 2) * rad;
    float bottom = -(height / 2) * rad;

    return glm::ortho(left, -left, bottom, -bottom, -100.f, 100.f);
}

glm::vec3 Camera::GetFront()
{
    return rotation * glm::vec3{0, 0, 1};
}

glm::vec3 Camera::GetUp()
{
    var Front = GetFront();

    glm::vec3 Right = normalize(cross(Front, glm::vec3{0, 1, 0}));
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower
    // movement.
    glm::vec3 Up = normalize(cross(Right, Front));

    return Up;
}

void Camera::redraw()
{
    if (renderTexture->viewId > 0)
    {

        tmgl::setViewRect(renderTexture->viewId, 0, 0, renderTexture->realTexture->width,
                          renderTexture->realTexture->height);
    }
    else
    {
        tmgl::setViewRect(renderTexture->viewId, 0, 0,
                          static_cast<uint16_t>(renderer->windowWidth), static_cast<uint16_t>(renderer->windowHeight));
    }


    setViewMode(renderTexture->viewId, bgfx::ViewMode::Sequential);

    var l = renderLayers;
    if (renderLayers == -1)
    {
        l = 0;
        for (auto layerMap : obj::LayerMask::layerMap)
        {
            l |= layerMap.second;
        }
    }

    /*
    for (const auto& draw : calls)
    {
        printf("Submitting draw call: Layer %d\n", draw.layer);
    }
    */

    for (int i = 0; i < drawCalls.size(); i++)
    {
        const var& call = drawCalls[i];
        // tmgl::setTransform(call.transformMatrix);

        if (!call.program)
            continue;

        if (!(l & call.renderLayer))
            continue;

        switch (call.matrixMode)
        {
            case MaterialState::ViewProj:
                tmgl::setViewTransform(renderTexture->viewId, value_ptr(GetView_m4()), value_ptr(GetProjection_m4()));
                break;
            case MaterialState::View:
                // tmgl::setViewTransform(0, mainCamera->GetView(), oneMat);
                break;
            case MaterialState::Proj:
                // tmgl::setViewTransform(0, oneMat, proj);
                break;
            case MaterialState::None:
                // tmgl::setViewTransform(0, oneMat, oneMat);
                break;
            case MaterialState::ViewOrthoProj:
                // tmgl::setViewTransform(0, mainCamera->GetView(), ortho);
                tmgl::setViewTransform(renderTexture->viewId, value_ptr(GetView_m4()),
                                       value_ptr(GetOrthoProjection_m4()));
                break;
            case MaterialState::OrthoProj:
            {
                // tmgl::setViewTransform(0, oneMat, ortho);
                setUniform(orthoHandle, value_ptr(GetOrthoProjection_m4()));
            }
            break;
        }

        setUniform(timeHandle, getTimeUniform());
        setUniform(vposHandle, math::vec4toArray(glm::vec4(position, 0)));

        if (call.animationMatrices.size() > 0)
        {
            // tmgl::setUniform(animHandle, call.animationMatrices);

            var matrixCount = call.animationMatrices.size() + 1;

            std::vector<float> matrixData;
            matrixData.reserve(matrixCount * 16);

            {
                const float* transformPtr = value_ptr(call.transformMatrix);
                matrixData.insert(matrixData.end(), transformPtr, transformPtr + 16);
            }

            for (const auto& full_vec : call.animationMatrices)
            {
                const float* matPtr = value_ptr(full_vec);
                matrixData.insert(matrixData.end(), matPtr, matPtr + 16);
            }


            // tmgl::setTransform(glm::value_ptr(fullVec[0]));
            tmgl::setTransform(matrixData.data(), static_cast<uint16_t>(matrixCount));
        }
        else
        {
            var matrix = call.transformMatrix;
            if (call.matrixMode == MaterialState::OrthoProj)
            {
            }
            tmgl::setTransform(value_ptr(matrix));
        }

        if (lights.size() > 0)
            lightUniforms->Apply(lights);

        if (call.mesh)
        {
            call.mesh->use();
        }
        else
        {
            setVertexBuffer(0, call.vbh, 0, call.vertexCount);
            setIndexBuffer(call.ibh, 0, call.indexCount);
        }

        tmgl::setState(call.state);

        call.program->Push(renderTexture->viewId, call.overrides, call.overrideCt);

        // tmgl::discard();
    }
}

Camera* Camera::GetMainCamera()
{
    return renderer->cameraCache[0];
}

Camera::Camera()
{
    renderer->cameraCache.push_back(this);
}

Camera::~Camera()
{
    renderer->cameraCache.erase(VEC_FIND(renderer->cameraCache, this));
    if (renderTexture)
    {
        delete renderTexture;
    }
}

Color Color::FromHex(string hex)
{
    if (hex.starts_with("#"))
    {
        hex = hex.substr(1);
    }

    int r, g, b;
    sscanf(hex.c_str(), "%02x%02x%02x", &r, &g, &b);

    Color c{};
    c.r = r / 255.0f;
    c.g = g / 255.0f;
    c.b = b / 255.0f;

    return c;
}

tmgl::VertexLayout Vertex::getVertexLayout()
{
    var layout = tmgl::VertexLayout();
    layout.begin()
          .add(tmgl::Attrib::Position, 3, tmgl::AttribType::Float)
          .add(tmgl::Attrib::Normal, 3, tmgl::AttribType::Float)
          .add(tmgl::Attrib::TexCoord0, 2, tmgl::AttribType::Float)
          .add(tmgl::Attrib::Indices, 4, tmgl::AttribType::Float, false, false)
          .add(tmgl::Attrib::Weight, 4, tmgl::AttribType::Float, true)
          .end();

    return layout;
}

void Vertex::SetBoneData(int boneId, float boneWeight)
{
    if (boneWeight <= 0.01)
        return;
    for (int i = 0; i < 4; ++i)
    {
        if (boneIds[i] < 0)
        {
            boneIds[i] = static_cast<s16>(boneId);
            boneWeights[i] = boneWeight;
            break;
        }
    }
}

float DrawCall::getDistance(Camera* cam)
{
    return glm::length(cam->position - sortedPosition);
}

MatrixArray tmt::render::GetMatrixArray(glm::mat4 m)
{
    MatrixArray mat(16, 0.0f);
    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            mat[x * 4 + y] = m[x][y];
        }
    }

    return mat;
}

Mesh* tmt::render::createMesh(Vertex* data, u16* indices, u32 vertCount, u32 triSize,
                              tmgl::VertexLayout layout, Model* model, string name)
{
    u32 stride = layout.getStride();
    u32 vertS = stride * vertCount;

    u32 indeS = sizeof(u16) * triSize;

    if (name == "none")
    {
        name = std::generateRandomString(12);
        name += std::to_string(vertCount);
        name += std::to_string(triSize);
    }

    var mesh = new Mesh();
    //memcpy(mesh->vertices, data, sizeof(data));
    //memcpy(mesh->indices, indices, sizeof(indices));
    mesh->indices = indices;
    mesh->vertices = data;
    mesh->model = model;
    mesh->name = name;
    if (model)
    {
        mesh->idx = model->meshes.size();
    }

    {
        const tmgl::Memory* mem = tmgl::alloc(vertS);

        bx::memCopy(mem->data, data, vertS);

        tmgl::VertexBufferHandle vbh = createVertexBuffer(mem, layout);
        mesh->vbh = vbh;
        mesh->vertexCount = vertCount;
    }

    {
        const tmgl::Memory* mem = tmgl::alloc(indeS);

        bx::memCopy(mem->data, indices, indeS);

        tmgl::IndexBufferHandle ibh = createIndexBuffer(mem);

        mesh->ibh = ibh;
        mesh->indexCount = triSize;
    }

    ResMgr->loaded_meshes[name] = mesh;

    return mesh;
}

void tmt::render::pushDrawCall(DrawCall d)
{

    u32 l1, l2;

    math::unpackU64ToU32(d.layer, l1, l2);

    d.renderLayer = l2;
    d.layer = l1;

    drawCalls.push_back(d);
}

void tmt::render::takeScreenshot(string path)
{
    if (path == "null")
    {
        path = "Screenshot.png";
    }
}

void tmt::render::pushLight(light::Light* light)
{
    lights.push_back(light);
}


RendererInfo* tmt::render::init(int width, int height)
{
    glfwSetErrorCallback(glfw_errorCallback);
    if (!glfwInit())
        return nullptr;

#ifndef TMGL_BGFX
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, application->name.c_str(), nullptr, nullptr);
    if (!window)
        return nullptr;

#ifndef TMGL_BGFX
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
#endif

    tmgl::Init init;


#ifdef WIN32
    init.platformData.nwh = glfwGetWin32Window(window);
    //init.platformData.ndt = glfwGetWin32Window(window);
#endif

    init.windowWidth = static_cast<uint32_t>(width);
    init.windowHeight = static_cast<uint32_t>(height);

    init.vendorId = TMGL_PCI_ID_NVIDIA;
#ifdef TMGL_BGFX
    init.type = bgfx::RendererType::Direct3D11;
#endif


#ifndef TMGL_BGFX
    init.platformData.window = window;
    init.platformData.proc = (GLADloadproc)glfwGetProcAddress;
    init.platformData.swap = (TMGLSwapProc)glfwSwapBuffers;
#endif

    tmgl::init(init);

    renderer = new RendererInfo();
    renderer->window = window;

    renderer->windowWidth = width;
    renderer->windowHeight = height;


    ShaderInitInfo info = {
        SubShader::CreateSubShader("test/vert", SubShader::Vertex),
        SubShader::CreateSubShader("test/frag", SubShader::Fragment),
        "$defaultShader"
    };

    defaultShader = Shader::CreateShader(info);

    std::vector<byte> whiteData;

    for (int x = 0; x < 10; ++x)
    {
        for (int y = 0; y < 10; ++y)
        {
            for (int c = 0; c < 3; ++c)
            {
                whiteData.push_back(0xFFF);
            }
        }
    }

    GetPrimitive(prim::Cube);
    GetPrimitive(prim::Sphere);
    GetPrimitive(prim::Quad);

    var white =
        new Texture(10, 10, tmgl::TextureFormat::RGB8, 0, tmgl::copy(whiteData.data(), whiteData.size()), "White");


    return renderer;
}

void tmt::render::update()
{

    u8 btn = ((input::Mouse::GetMouseButton(input::Mouse::Left, true) == input::Mouse::Hold) ? IMGUI_MBUT_LEFT : 0) |
        ((input::Mouse::GetMouseButton(input::Mouse::Right, true) == input::Mouse::Hold) ? IMGUI_MBUT_RIGHT : 0) |
        ((input::Mouse::GetMouseButton(input::Mouse::Middle, true) == input::Mouse::Hold) ? IMGUI_MBUT_MIDDLE : 0);
    glfwGetWindowSize(renderer->window, &renderer->windowWidth, &renderer->windowHeight);


    if (renderer->useImgui)
    {
        tmgl::imgui::beginFrame(mousep.x, mousep.y, btn, input::Mouse::GetMouseScroll().y,
                                static_cast<u16>(renderer->windowWidth), static_cast<u16>(renderer->windowHeight),
                                input::GetLastKey());
        {

            for (auto debug_func : debugFuncs)
            {
                debug_func();
            }
        }

        tmgl::imgui::endFrame();
    }


    if (!subHandlesLoaded)
    {

        lightUniforms = new light::LightUniforms;

        orthoHandle = createUniform("iu_ortho", tmgl::UniformType::Mat4);
        timeHandle = createUniform("iu_time", tmgl::UniformType::Vec4);
        vposHandle = createUniform("iu_viewPos", tmgl::UniformType::Vec4);
        animHandle = createUniform("iu_boneMatrices", tmgl::UniformType::Mat4, MAX_BONE_MATRICES);

        subHandlesLoaded = true;
    }

    std::sort(drawCalls.begin(), drawCalls.end(),
              [](const DrawCall& a, const DrawCall& b) { return a.layer < b.layer; });

    for (auto cameraCache : renderer->cameraCache)
    {
        cameraCache->redraw();
    }

    for (auto drawCall : drawCalls)
    {
        drawCall.clean();
    }

    drawCalls.clear();

    debugCalls.clear();
    debugFuncs.clear();
    lights.clear();

    frameTime = tmgl::frame();
    lastKey = -1;

    glfwPollEvents();
    counterTime++;


    //tmgl::touch(0);
    // tmgl::touch(1);

    if (input::Keyboard::GetKey(GLFW_KEY_LEFT_CONTROL) == input::Keyboard::Press)
    {
        fs::ResourceManager::pInstance->ReloadShaders();
    }

}

void DrawCall::clean()
{
    delete[] overrides;
}

void tmt::render::shutdown()
{
    tmgl::shutdown();
    glfwTerminate();
}


void tmgl::setUniform(UniformHandle handle, glm::vec4 v)
{
    setUniform(handle, tmt::math::vec4toArray(v));
}

void tmgl::setUniform(UniformHandle handle, std::vector<glm::vec4> v)
{
    auto arr = new float[v.size() * 4];
    for (int i = 0; i < v.size() * 4; i += 4)
    {
        var vec = v[i / 4];
        for (int j = 0; j < 4; ++j)
        {
            arr[i + j] = vec[j];
        }
    }

    setUniform(handle, arr, v.size());

    delete[] arr;
}

void tmgl::setUniform(UniformHandle handle, std::vector<glm::mat4> v)
{
    var arr = tmt::math::mat4ArrayToArray(v);
    setUniform(handle, arr, v.size());

    delete[] arr;
}
