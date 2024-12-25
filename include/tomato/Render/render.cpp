#include "render.hpp" 
#include "globals.hpp"
#include "vertex.h"
#include "common/imgui/imgui.h"

#define ResMgr tmt::fs::ResourceManager::pInstance

using namespace tmt::render;

RendererInfo* RendererInfo::GetRendererInfo()
{ return renderer; }

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

tmt::render::ShaderUniform::~ShaderUniform()
{ bgfx::destroy(handle); }

tmt::render::SubShader::SubShader(string name, ShaderType type)
{

    this->name = name;
    this->type = type;
    Reload();

    fs::ResourceManager::pInstance->loaded_sub_shaders[name] = this;
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

void SubShader::Reload()
{
    if (isLoaded)
    {
        bgfx::destroy(handle);
        uniforms.clear();
    }
    isLoaded = true;

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

    bgfx::setName(handle, name.c_str());
}

tmt::render::SubShader::~SubShader()
{
    bgfx::destroy(handle);
    for (auto uniform : uniforms)
    {
        delete uniform;
    }
}

tmt::render::SubShader* tmt::render::SubShader::CreateSubShader(string name, ShaderType type)
{
    if (IN_VECTOR(fs::ResourceManager::pInstance->loaded_sub_shaders, name))
    {
        return ResMgr->loaded_sub_shaders[name];
    }

    return new SubShader(name, type);
}

tmt::render::Shader::Shader(ShaderInitInfo info)
{
    program = createProgram(info.vertexProgram->handle, info.fragmentProgram->handle, true);

    subShaders.push_back(info.vertexProgram);
    subShaders.push_back(info.fragmentProgram);

    ResMgr->loaded_shaders[info.name] = this;
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

tmt::render::Shader::~Shader()
{
    bgfx::destroy(program);
    for (var shader : subShaders)
    {
        delete shader;
    }
}

void Shader::Reload()
{
    if (bgfx::isValid(program))
    {
            bgfx::destroy(program);
    }

    for (auto sub_shader : subShaders)
    {
        sub_shader->Reload();
    }

    program = bgfx::createProgram(subShaders[0]->handle, subShaders[1]->handle, true);
}

tmt::render::Shader* tmt::render::Shader::CreateShader(ShaderInitInfo info)
{
    if (info.name == "UNDEFINED")
    {
        std::hash<string> hsh;
        info.name = hsh(info.fragmentProgram->name + "_"+info.vertexProgram->name);
    }

    if (IN_VECTOR(ResMgr->loaded_shaders, info.name))
    {
       return ResMgr->loaded_shaders[info.name];
    }

    return new Shader(info);
}

tmt::render::ComputeShader::ComputeShader(SubShader *shader)
{
    program = createProgram(shader->handle, true);
    internalShader = shader;

    ResMgr->loaded_compute_shaders[shader->name + "_CMP"] = this;
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

tmt::render::ComputeShader::~ComputeShader()
{
    bgfx::destroy(program);
    delete internalShader;
}

tmt::render::ComputeShader* tmt::render::ComputeShader::CreateComputeShader(SubShader* shader)
{
    if (IN_VECTOR(ResMgr->loaded_compute_shaders, shader->name+"_CMP"))
    {
        return ResMgr->loaded_compute_shaders[shader->name + "_CMP"];
    }

    return new ComputeShader(shader);
}

tmt::render::MaterialOverride *tmt::render::Material::GetUniform(string name, bool force)
{
    for (auto override : overrides)
        if (override->name == name)
            return override;

    if (force)
    {
        var ovr = new MaterialOverride();
        ovr->name = name;

        overrides.push_back(ovr);

        return ovr;
    }

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

tmt::render::Material::~Material()
{ delete[] overrides.data(); }

tmt::render::Mesh::~Mesh()
{
    bgfx::destroy(ibh);
    bgfx::destroy(vbh);
    for (auto vertex_buffer : vertexBuffers)
    {
        bgfx::destroy(vertex_buffer);
    }
    bgfx::destroy(indexBuffer);

    delete[] vertices;
    delete[] indices;
}

void tmt::render::Mesh::use()
{
    setVertexBuffer(0, vbh, 0, vertexCount);
    bgfx::setIndexBuffer(ibh, 0, indexCount);
}

void tmt::render::Mesh::draw(glm::mat4 transform, Material *material, std::vector<glm::mat4> anims)
{
    var drawCall = DrawCall();

    drawCall.mesh = this;
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = material->state.matrixMode;

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
        drawCall.overrides = material->overrides.data();
        drawCall.overrideCt = material->overrides.size();
    }
    else
        drawCall.overrides = nullptr;

    pushDrawCall(drawCall);
}

tmt::render::Texture* tmt::render::Model::GetTextureFromName(string name)
{

    for (Texture* tex : textures)
    {
        if (tex->name == name)
            return tex;
    }

    return nullptr;
}

tmt::render::Material* tmt::render::Model::CreateMaterial(int index, Shader* shader)
{
    return CreateMaterial(materials[index], shader);
}

tmt::render::Material* tmt::render::Model::CreateMaterial(MaterialDescription* materialDesc, Shader* shader)
{
    var material = new Material(shader);

    material->GetUniform("u_color", true)->v4 = Color::White.getData();

    for (auto [uniform, texture] : materialDesc->Textures)
    {
        material->GetUniform(uniform, true)->tex = GetTextureFromName(texture);
    }

    return material;
}

tmt::render::Animation* tmt::render::Model::GetAnimation(string name)
{
    for (auto animation : animations)
    {
        if (animation->name == name)
            return animation;
    }
    return nullptr;
}

void tmt::render::SceneDescription::Node::SetParent(Node* parent)
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

void tmt::render::SceneDescription::Node::AddChild(Node* child)
{ child->SetParent(this); }

tmt::render::SceneDescription::Node::Node(fs::BinaryReader* reader, SceneDescription* scene)
{
    name = reader->ReadString();

    position = reader->ReadVec3();

    var x = position.x;
    var z = position.z;
    position.x = z;
    position.z = x;

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

    isBone = reader->ReadInt32() == 0 ? true : false;

    var childCount = reader->ReadInt32();

    for (int i = 0; i < childCount; ++i)
    {
        children.push_back(new Node(reader, scene));
    }
}

tmt::render::SceneDescription::Node::Node(aiNode* node, SceneDescription* scene)
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

tmt::render::SceneDescription::Node* tmt::render::SceneDescription::Node::GetNode(string name)
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

tmt::render::SceneDescription::Node* tmt::render::SceneDescription::GetNode(string name)
{
    return rootNode->GetNode(name);
}

tmt::render::SceneDescription::Node* tmt::render::SceneDescription::Node::GetNode(aiNode* node)
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

tmt::render::Model* tmt::render::SceneDescription::GetModel(string name)
{
    for (auto model : models)
    {
        if (model->name == name)
            return model;
    }

    return nullptr;
}

tmt::render::Mesh* tmt::render::SceneDescription::GetMesh(int idx)
{
    var offset = 0;
    for (Model* model : models)
    {
        if (idx >= model->meshes.size()+offset)
        {
            offset += model->meshes.size();
            continue;
        }

        return model->meshes[idx - offset];
    }

    return nullptr;
}

tmt::render::SceneDescription::Node* tmt::render::SceneDescription::GetNode(aiNode* node)
{
    return rootNode->GetNode(node);
}

std::vector<tmt::render::SceneDescription::Node*> tmt::render::SceneDescription::GetAllChildren()
{
    return rootNode->GetAllChildren();
}

tmt::obj::Object* tmt::render::SceneDescription::Node::ToObject(int modelIndex)
{
    var obj = new tmt::obj::Object();

    obj->name = name;
    obj->position = position;
    obj->rotation = (rotation);
    obj->scale = scale;

    for (int mesh_index : meshIndices)
    {
        var mesh = scene->GetMesh(mesh_index);
        var mshObj = tmt::obj::MeshObject::FromBasicMesh(mesh);
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

std::vector<tmt::render::SceneDescription::Node*> tmt::render::SceneDescription::Node::GetAllChildren()
{
    var c = std::vector<Node*>(children);

    for (auto child : children)
    {
        var c2 = child->GetAllChildren();
        c.insert(c.end(), c2.begin(), c2.end());
    }

    return c;

}

tmt::render::SceneDescription::SceneDescription(string path)
{
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
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_PopulateArmatureData | aiProcess_ValidateDataStructure);

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

        {
            var model = new Model(scene, this);
            models.push_back(model);
        }

        import.FreeScene();
    }
}

tmt::obj::Object* tmt::render::SceneDescription::ToObject()
{

    return rootNode->ToObject();
}

tmt::render::SceneDescription::~SceneDescription()
{
    delete[] models.data();
}

int tmt::render::BoneObject::Load(SceneDescription::Node* node, int count)
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
            count++;
            count = bone->Load(child, count);
            bone->SetParent(this);
        }
    }

    return count;
}

glm::mat4 tmt::render::BoneObject::GetGlobalOffsetMatrix()
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
        transformation =
        glm::translate(glm::mat4(1.0), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0), scale);
    }
return transformation;
}

glm::mat4 tmt::render::BoneObject::GetOffsetMatrix() {
    glm::mat4 offset(1.0);
    if (bone != nullptr)
    {
        offset = bone->skeleton->boneInfoMap[name].offset;
    }
    return offset;
}

void tmt::render::SkeletonObject::Load(SceneDescription::Node* node)
{
    name = node->name;
    var ct = -1;
    for (auto child : node->children)
    {
        if (child->isBone)
        {
            var bone = new BoneObject();
            bone->name = child->name;
            bone->position = child->position;
            bone->rotation = (child->rotation);
            bone->scale = child->scale;
            ct++;
            ct = bone->Load(child, ct);
            bone->SetParent(this);
        }
    }

    bones = GetObjectsFromType<BoneObject>();
}

tmt::render::BoneObject* tmt::render::SkeletonObject::GetBone(string name)
{
    for (auto bone : bones)
    {
        if (bone->name == name)
            return bone;
    }

    return nullptr;
}

bool tmt::render::SkeletonObject::IsSkeletonBone(BoneObject* bone)
{
    return std::find(bones.begin(), bones.end(), bone) != bones.end();
}


Animator::Animator()
{

}

glm::mat4 tmt::render::Animator::AnimationBone::Update(float animationTime)
{
    var translation = InterpolatePosition(animationTime);
    var rotation = InterpolateRotation(animationTime);
    var scale = InterpolateScaling(animationTime);
    localTransform = translation * rotation * scale;

    return localTransform;
}

int tmt::render::Animator::AnimationBone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < channel->positions.size() - 1; ++index)
    {
        if (animationTime <= channel->positions[index + 1]->time)
            return index;
    }
    assert(0);
}

int tmt::render::Animator::AnimationBone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < channel->rotations.size() - 1; ++index)
    {
        if (animationTime <= channel->rotations[index + 1]->time)
            return index;
    }
    assert(0);
}

int tmt::render::Animator::AnimationBone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < channel->scales.size() - 1; ++index)
    {
        if (animationTime <= channel->scales[index + 1]->time)
            return index;
    }
    assert(0);
}

float tmt::render::Animator::AnimationBone::GetScaleFactor(float lasttime, float nexttime, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lasttime;
    float framesDiff = nexttime - lasttime;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 tmt::render::Animator::AnimationBone::InterpolatePosition(float animationTime)
{
    if (1 == channel->positions.size())
    {
        var finalPosition = channel->positions[0]->value;
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->positions[p0Index]->time, channel->positions[p1Index]->time, animationTime);
    glm::vec3 finalPosition =
        glm::mix(channel->positions[p0Index]->value, channel->positions[p1Index]->value, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 tmt::render::Animator::AnimationBone::InterpolateRotation(float animationTime)
{
    if (1 == channel->rotations.size())
    {
        auto finalRotation = glm::normalize(channel->rotations[0]->value);
        return glm::toMat4(finalRotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->rotations[p0Index]->time, channel->rotations[p1Index]->time, animationTime);
    glm::quat finalRotation =
        glm::slerp(channel->rotations[p0Index]->value, channel->rotations[p1Index]->value, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 tmt::render::Animator::AnimationBone::InterpolateScaling(float animationTime)
{
    if (1 == channel->scales.size())
    {
        var finalScale = channel->scales[0]->value;
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->scales[p0Index]->time, channel->scales[p1Index]->time, animationTime);
    glm::vec3 finalScale = glm::mix(channel->scales[p0Index]->value, channel->scales[p1Index]->value, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
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

void tmt::render::BoneObject::CalculateBoneMatrix(SkeletonObject* skeleton, const glm::mat4 parentTransform)
{
    if (bone == nullptr)
    {
        if (skeleton->skeleton != nullptr)
        {
            bone = skeleton->skeleton->GetBone(name);
        }
    }

    string nodeName = bone->name;
    glm::mat4 nodeTransform = bone->transformation;

    nodeTransform = GetLocalTransform();

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = skeleton->skeleton->boneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        skeleton->boneMatrices[index] = globalTransformation * offset;
    }

    debug::Gizmos::matrix = globalTransformation;
    debug::Gizmos::DrawSphere(glm::vec3{0}, 0.1f);

    for (Object* child : children)
    {
        var boneChild = child->Cast<BoneObject>();
        boneChild->CalculateBoneMatrix(skeleton, globalTransformation);
    }
}

void tmt::render::Animator::Update()
{
    if (!skeleton)
    {
        skeleton = parent->GetObjectFromType<SkeletonObject>();
    }

    if (currentAnimation && skeleton) 
    {
        time += (flt currentAnimation->ticksPerSecond) * (deltaTime);
        time = fmod(time, currentAnimation->duration);
        if (currentAnimation->duration <= 0)
            time = 0;

        if (animationBones.size() <= currentAnimation->nodeChannels.size())
            LoadAnimationBones();

        pushBoneMatrices.clear();
        pushBoneMatrices.resize(MAX_BONE_MATRICES, glm::mat4(1.0));

        for (auto animation_bone : animationBones)
        {
            animation_bone->Update(time);

            skeleton->bones[animation_bone->boneId]->SetTransform(animation_bone->localTransform);
        }

        //CalculateBoneTransform(skeleton->skeleton->GetBone(skeleton->skeleton->rootName), glm::mat4(1.0));

        //skeleton->boneMatrices = pushBoneMatrices;
    }


    Object::Update();
}

void tmt::render::Animator::LoadAnimationBones()
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

void tmt::render::SkeletonObject::Update()
{

    boneMatrices.clear();
    boneMatrices.resize(MAX_BONE_MATRICES, glm::mat4(1.0));

    //GetBone(skeleton->rootName)->CalculateBoneMatrix(this, glm::mat4(1.0));
    CalculateBoneTransform(skeleton->GetBone(skeleton->rootName), glm::mat4(1.0));

    Object::Update();
}

void SkeletonObject::CalculateBoneTransform(const Skeleton::Bone* skeleBone, glm::mat4 parentTransform)
{
    string nodeName = skeleBone->name;
    glm::mat4 nodeTransform = skeleBone->transformation;
    int idx = 0;

    var boneInfoMap = skeleton->boneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        idx = boneInfoMap[nodeName].id;
    }

    BoneObject* animBone = bones[idx];

    if (animBone)
    {
        // animBone->Update(time);
        nodeTransform = bones[idx]->GetLocalTransform();
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        var index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        boneMatrices[index] = globalTransform * offset;
    }

    debug::Gizmos::matrix = globalTransform;
    debug::Gizmos::DrawSphere(glm::vec3{0}, 0.1f);

    for (string child : skeleBone->children)
    {
        var cbone = skeleton->GetBone(child);
        CalculateBoneTransform(cbone, globalTransform);
    }
}

void Animator::CalculateBoneTransform(const Skeleton::Bone* skeleBone, glm::mat4 parentTransform)
{
    string nodeName = skeleBone->name;
    glm::mat4 nodeTransform = skeleBone->transformation;

    AnimationBone* animBone = GetBone(nodeName);

    if (animBone)
    {
        //animBone->Update(time);
        nodeTransform = skeleton->bones[animBone->boneId]->GetLocalTransform();
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    var boneInfoMap = skeleton->skeleton->boneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        var index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        pushBoneMatrices[index] = globalTransform * offset;
    }

    debug::Gizmos::matrix = globalTransform;
    debug::Gizmos::DrawSphere(glm::vec3{0}, 0.1f);

    for (string child : skeleBone->children)
    {
        var cbone = skeleton->skeleton->GetBone(child);
        CalculateBoneTransform(cbone, globalTransform);
    }
}


tmt::render::Model::Model(string path)
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
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                   aiProcess_FindInvalidData | aiProcess_PreTransformVertices | aiProcess_PopulateArmatureData | aiProcess_GenUVCoords );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        LoadFromAiScene(scene);
    }
}

tmt::render::Skeleton::Bone* tmt::render::Skeleton::GetBone(string name)
{
    for (auto value : bones)
    {
        if (value->name == name)
            return value;
    }

    return nullptr;
}

tmt::render::Model::Model(const aiScene *scene)
{ LoadFromAiScene(scene); }

tmt::render::Model::Model(const aiScene* scene, SceneDescription* description)
{ LoadFromAiScene(scene, description); }

void tmt::render::Model::LoadFromAiScene(const aiScene* scene, SceneDescription* description)
{
    name = "TMDL_"+std::string(scene->mName.C_Str());
    skeleton = new Skeleton();

    skeleton->inverseTransform = tmt::math::convertMat4(scene->mRootNode->mTransformation.Inverse());


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

            if (skeleton->boneInfoMap.find(boneName) == skeleton->boneInfoMap.end())
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
            }

            if (skeleton->GetBone(b->mName.C_Str()) == nullptr)
            {

                bone = new Skeleton::Bone;
                bone->name = boneName;
                bone->skeleton = skeleton;

                skeleton->bones.push_back(bone);
            }
            else
            {
                bone = skeleton->GetBone(b->mName.C_Str());
            }

            for (int k = 0; k < b->mNumWeights; ++k)
            {
                var weight = b->mWeights[k];
                bone->weights.push_back(Skeleton::Bone::VertexWeight{weight.mVertexId, weight.mWeight});
                vertices[weight.mVertexId].SetBoneData(boneId, weight.mWeight);
            }

            if (description)
            {

                if (j == 0)
                {
                    var anode = description->GetNode(b->mArmature->mName.C_Str());

                    skeleton->rootName = b->mName.C_Str();
                    if (anode != description->rootNode)
                    {
                        anode->SetParent(modelNode);
                    }
                }

                var bnode = description->GetNode(b->mNode);
                if (bnode)
                    bnode->isBone = true;
            }
        }

        Vertex* verts = new Vertex[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), verts);

        u16* incs = new u16[indices.size()];
        std::copy(indices.begin(), indices.end(), incs);

        var mesh = createMesh(verts, incs, vertices.size(), indices.size(), Vertex::getVertexLayout(), this);

        meshes.push_back(mesh);
        materialIndices.push_back(msh->mMaterialIndex);



        if (description) {
            var mnd = description->GetNode(msh->mName.C_Str());

            var meshNode = new SceneDescription::Node;
            meshNode->name = msh->mName.C_Str();

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

    for (int i = 0; i < scene->mNumMaterials; ++i)
    {
        var mat = scene->mMaterials[i];

        var desc = new MaterialDescription;

        desc->Name = mat->GetName().C_Str();

        for (int i = 0; i < AI_TEXTURE_TYPE_MAX; ++i)
        {
            var type = static_cast<aiTextureType>(i);
            for (int j = 0; j < mat->GetTextureCount(type); ++j)
            {
                aiString path;
                aiTextureMapping mapping;
                unsigned int uvindex;
                float blend;
                aiTextureOp op;
                aiTextureMapMode mapmode[3];

                aiReturn result = mat->GetTexture(type, j, &path, &mapping, &uvindex, &blend, &op, mapmode);

                if (result == aiReturn_SUCCESS)
                {
                    //desc->Textures.insert(std::make_pair(type, path.C_Str()));
                }
            }
        }


        materials.push_back(desc);
    }

    for (int i = 0; i < scene->mNumAnimations; ++i)
    {
        var anim = scene->mAnimations[i];

        var animation = new Animation();
        animation->name = anim->mName.C_Str();
        animation->duration = flt anim->mDuration;
        animation->ticksPerSecond = static_cast<int>(anim->mTicksPerSecond);

        for (int i = 0; i < anim->mNumChannels; ++i)
        {
            var channel = anim->mChannels[i];
            var nodeChannel = new Animation::NodeChannel;
            nodeChannel->name = channel->mNodeName.C_Str();

            for (int i = 0; i < channel->mNumPositionKeys; ++i)
            {
                var posKey = channel->mPositionKeys[i];
                nodeChannel->positions.push_back(new Animation::NodeChannel::NodeKey<glm::vec3>(static_cast<float>(posKey.mTime), math::convertVec3(posKey.mValue)));
            }

            for (int i = 0; i < channel->mNumScalingKeys; ++i)
            {
                var posKey = channel->mScalingKeys[i];
                nodeChannel->scales.push_back(new Animation::NodeChannel::NodeKey<glm::vec3>(static_cast<float>(posKey.mTime), math::convertVec3(posKey.mValue)));
            }

            for (int i = 0; i < channel->mNumRotationKeys; ++i)
            {
                var posKey = channel->mRotationKeys[i];
                nodeChannel->rotations.push_back(new Animation::NodeChannel::NodeKey<glm::quat>(static_cast<float>(posKey.mTime), math::convertQuat(posKey.mValue)));
            }

            animation->nodeChannels.push_back(nodeChannel);
        }

        animations.push_back(animation);
    }

    for (int i = 0; i < scene->mNumSkeletons; ++i)
    {
        var skl = scene->mSkeletons[i];
        

        break;
    }

    var children = description->GetAllChildren();
    std::vector<Skeleton::Bone*> n_bones;
    int nodeCount = 0;
    for (auto child : children)
    {
        child->id = nodeCount;
        if (child->isBone)
        {
            var bone = skeleton->GetBone(child->name);
            if (skeleton->boneInfoMap.contains(bone->name))
            {
                n_bones.push_back(bone);
            }
        }
        nodeCount++;
    }
    skeleton->bones = n_bones;

    for (auto child : children)
    {
        if (child->isBone)
        {
            var bone = skeleton->GetBone(child->name);
            if (bone)
            {
                var parentBone = skeleton->GetBone(child->parent->name);
                if (parentBone)
                {
                    parentBone->children.push_back(bone->name);
                }
            }
        }
    }

    if (description)
        modelNode->SetParent(description->rootNode);

}

tmt::render::Model::~Model()
{
    delete[] meshes.data();
    delete[] textures.data();
    delete[] materials.data();
    delete[] animations.data();

    delete skeleton;
}

tmt::render::Model::Model(fs::BinaryReader* reader, SceneDescription* description)
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

                Vertex vtx = {p, n, u, math::convertIVec4(boneIds, boneIdxCount), math::convertVec4(weights, weightCount)};
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

            var format = bgfx::TextureFormat::RGBA8;

            switch (channelCount)
            {
                case 1:
                    format = bgfx::TextureFormat::R8;
                    break;
                case 2:
                    format = bgfx::TextureFormat::RG8;
                    break;
                case 3:
                    format = bgfx::TextureFormat::RGB8;
                    break;
                default:
                    break;
            }

            var tex = new Texture(width, height, format, BGFX_SAMPLER_UVW_CLAMP, bgfx::copy(dataV.data(), size), name);


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
    for (int i = 0; i < animCount; ++i)
    {
        animations.push_back(new Animation(reader));
    }
}

tmt::render::Animation::Animation(fs::BinaryReader* reader)
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

tmt::render::Skeleton::Skeleton(fs::BinaryReader* reader)
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
        bone->rotation = rot;
        bone->scale = scl;

        glm::mat4 offsetMatrix;

        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                offsetMatrix[x][y] = reader->ReadSingle();
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
}

tmt::obj::Object* tmt::render::Model::CreateObject(Shader* shdr)
{

    var shader = shdr;

    if (shader == nullptr)
    {
        shader = defaultShader;
    }

    var mdlObj = new obj::Object();

    std::vector<Material*> mats;

    
    for (auto materialDesc : this->materials)
    {
        mats.push_back(CreateMaterial(materialDesc, shader));
    }
    

    var midx = 0;
    for (auto value : meshes)
    {
        var mshObj = tmt::obj::MeshObject::FromBasicMesh(value);
        mshObj->material = mats[materialIndices[midx]];
        mshObj->SetParent(mdlObj);

        midx++;
    }

    return mdlObj;
}

tmt::render::Texture::Texture(string path, bool isCubemap)
{

    uint64_t textureFlags = BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR | BGFX_SAMPLER_POINT; // Adjust as needed
    if (!isCubemap)
    {
        int nrChannels;
        u8* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        var dataSize = width * height * nrChannels;
        unsigned char* rgbaData = new unsigned char[dataSize];

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
            for (int i = 0; i < width * height; ++i)
            {
                rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
                rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
                rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
                rgbaData[i * 4 + 3] = data[i * nrChannels + 3]; // Add alpha channel with value 1.0
            }
        }
        
        

        bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA8;

        // Create the texture in bgfx, passing the image data directly
        handle = bgfx::createTexture2D(static_cast<u16>(width), static_cast<u16>(height), false, 1, textureFormat,
                                       textureFlags, bgfx::copy(data, dataSize));
        format = textureFormat;

        
        stbi_image_free(data);
    }
    else
    {

        int nrChannels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);

        bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::RGBA32F;

        u32 dataSize = width * height * 4;
        float* rgbaData = new float[dataSize];
        for (int i = 0; i < width * height; ++i)
        {
            rgbaData[i * 4 + 0] = data[i * nrChannels + 0];
            rgbaData[i * 4 + 1] = data[i * nrChannels + 1];
            rgbaData[i * 4 + 2] = data[i * nrChannels + 2];
            rgbaData[i * 4 + 3] = 1.0f; // Add alpha channel with value 1.0
        }

        u16 uwidth = width;
        u16 uheight = height;


        var size = 512;
        // Create the texture in bgfx, passing the image data directly
        var temp_handle = createTexture2D(uwidth, uheight,
                                          false, // no mip-maps
                                          1, // single layer
                                          textureFormat, BGFX_SAMPLER_UVW_CLAMP,
                                          bgfx::copy(rgbaData, dataSize * sizeof(float)) // copies the image data
        );
        bgfx::setName(temp_handle, "basecbtex");

        //var colorBuffer = new tmt::render::Texture(size, size, bgfx::TextureFormat::RGBA32F,BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_POINT);

        var readBackShader = Shader::CreateShader(tmt::render::ShaderInitInfo{
            SubShader::CreateSubShader("equi/vert", tmt::render::SubShader::Vertex),
            SubShader::CreateSubShader("equi/frag", tmt::render::SubShader::Fragment),
        });



        var cubemapHandle = bgfx::createTextureCube(size, false, 1, bgfx::TextureFormat::RGBA16F,
                                                    BGFX_SAMPLER_UVW_CLAMP | BGFX_TEXTURE_BLIT_DST);
        bgfx::setName(cubemapHandle, "realcbtex");

        var cubeMapRenderTexture =
            new RenderTexture(size, size, bgfx::TextureFormat::RGBA16F, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

        var vid = cubeMapRenderTexture->vid;

        bgfx::setViewClear(vid, BGFX_CLEAR_DEPTH);
        bgfx::setViewRect(vid, 0, 0, size, size);
        bgfx::setViewFrameBuffer(vid, cubeMapRenderTexture->handle);
        bgfx::touch(vid);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        for (uint8_t i = 0; i < 6; ++i)
        {
            var view = captureViews[i];
            var proj = captureProjection;
            bgfx::setViewTransform(vid, math::mat4ToArray(view), math::mat4ToArray(proj));

            prim::GetPrimitive(prim::Cube)->use();

            bgfx::setTexture(0, bgfx::createUniform("s_texCube", bgfx::UniformType::Sampler), temp_handle);
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
            //bgfx::setTransform(math::mat4ToArray(glm::scale(glm::vec3{2})));

            readBackShader->Push(vid);

            bgfx::touch(vid);
            
            bgfx::blit(vid, cubemapHandle, 0, 0, 0, i, cubeMapRenderTexture->realTexture->handle, 0, 0, 0, 0, size, size);

            //break;
        }


        handle = cubemapHandle;

        
        stbi_image_free(data);
    }

}

tmt::render::Texture::Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory *mem, string name)
{
    handle = createTexture2D(width, height, false, 1, tf, flags, mem);

    bgfx::setName(handle, name.c_str());

    format = tf;
    this->width = width;
    this->height = height;
    this->name = name;
}

tmt::render::Texture::~Texture()
{ bgfx::destroy(handle); }

tmt::render::RenderTexture::RenderTexture(u16 width, u16 height, bgfx::TextureFormat::Enum format, u16 cf)
{
    const bgfx::Memory *mem = nullptr;
    if (format == bgfx::TextureFormat::RGBA8)
    {
        // std::vector<GLubyte> pixels(width * height * 4, (GLubyte)0xffffffff);
        // mem = bgfx::copy(pixels.data(), width * height* 4);
    }

    realTexture = new Texture(width, height, format,BGFX_TEXTURE_RT,mem);

    handle = createFrameBuffer(1, &realTexture->handle, true);
    this->format = format;

    bgfx::setViewName(vid, "RenderTexture");
    bgfx::setViewClear(vid, cf);
    bgfx::setViewRect(vid, 0, 0, width, height);
    setViewFrameBuffer(vid, handle);

    
}

tmt::render::RenderTexture::~RenderTexture()
{
    bgfx::destroy(handle);
    delete realTexture;
    bgfx::resetView(vid);
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

    bx::mtxProj(proj, glm::radians(FOV), static_cast<float>(renderer->windowWidth) / static_cast<float>(renderer->windowHeight),
                NearPlane,FarPlane , bgfx::getCaps()->homogeneousDepth);
    return proj;
}

glm::mat4 tmt::render::Camera::GetView_m4()
{ return glm::lookAt(position, position + GetFront(), GetUp()); }

glm::mat4 tmt::render::Camera::GetProjection_m4()
{
    return glm::perspective(glm::radians(FOV),
                            static_cast<float>(renderer -> windowWidth) / static_cast<float>(renderer->windowHeight), NearPlane, FarPlane);
}

glm::vec3 tmt::render::Camera::GetFront()
{
    return rotation * glm::vec3{0,0,1};
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
        .add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Int16, false, true)
        .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)
    .end();

    return layout;
}

void tmt::render::Vertex::SetBoneData(int boneId, float boneWeight)
{
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

tmt::render::MatrixArray tmt::render::GetMatrixArray(glm::mat4 m)
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

tmt::render::Mesh *tmt::render::createMesh(Vertex *data, u16 *indices, u32 vertCount, u32 triSize,
                                           bgfx::VertexLayout layout, Model* model)
{
    u32 stride = layout.getStride();
    u32 vertS = stride * vertCount;

    u32 indeS = sizeof(u16) * triSize;

    var mesh = new Mesh();
    //memcpy(mesh->vertices, data, sizeof(data));
    //memcpy(mesh->indices, indices, sizeof(indices));
    mesh->indices = indices;
    mesh->vertices = data;
    mesh->model = model;
    if (model)
    {
        mesh->idx = model->meshes.size();
    }

    {
        const bgfx::Memory *mem = bgfx::alloc(vertS);

        bx::memCopy(mem->data, data, vertS);

        bgfx::VertexBufferHandle vbh = createVertexBuffer(mem, layout);
        mesh->vbh = vbh;
        mesh->vertexCount = vertCount;

        if (vertCount == 4055)
        {
            var mtest = 4055 * 32;
            if (vertS != mtest)
            {
                std::cout << "multiplication error" << std::endl;
            }

            bgfx::setName(vbh, "test");
        }

        auto positions = new glm::vec3[vertCount];
        auto normals = new glm::vec3[vertCount];
        auto uvs = new glm::vec2[vertCount];

        for (int i = 0; i < vertCount; ++i)
        {
            var vertex = data[i];
            positions[i] = vertex.position;
            normals[i] = vertex.normal;
            uvs[i] = vertex.uv0;
            if (i == 116)
            {
                std::cout << std::endl;
            }
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

        bgfx::IndexBufferHandle ibh = createIndexBuffer(mem);

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

void tmt::render::pushLight(light::Light* light)
{ lights.push_back(light); }

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
        SubShader::CreateSubShader("test/vert", SubShader::Vertex),
        SubShader::CreateSubShader("test/frag", SubShader::Fragment),
    };

    defaultShader = Shader::CreateShader(info);

    var rdoc = bgfx::loadRenderDoc();

    if (renderer->useImgui)
        imguiCreate();

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

    u8 btn = ((input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == input::Mouse::Hold) ? IMGUI_MBUT_LEFT : 0) |
        ((input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == input::Mouse::Hold) ? IMGUI_MBUT_RIGHT : 0) |
        ((input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == input::Mouse::Hold) ? IMGUI_MBUT_MIDDLE : 0);
    glfwGetWindowSize(renderer->window, &renderer->windowWidth, &renderer->windowHeight);

    if (renderer->useImgui)
    {

        imguiBeginFrame(mousep.x, mousep.y, btn, 0, (u16)renderer->windowWidth, (u16)renderer->windowHeight);

        {
            for (auto debug_func : debugFuncs)
            {
                debug_func();
            }
        }

        imguiEndFrame();
    }
    

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

        lightUniforms = new light::LightUniforms;

        orthoHandle = createUniform("iu_ortho", bgfx::UniformType::Mat4);
        timeHandle = createUniform("iu_time", bgfx::UniformType::Vec4);
        vposHandle = createUniform("iu_viewPos", bgfx::UniformType::Vec4);
        animHandle = createUniform("iu_boneMatrices", bgfx::UniformType::Mat4, MAX_BONE_MATRICES);

        subHandlesLoaded = true;
    }

    float t[4] = {static_cast<float>(counterTime), static_cast<float>(glm::sin(counterTime)),
                  static_cast<float>(glm::cos(counterTime)), flt renderer->usePosAnim};
    float d[4] = {(float)lights.size(), 0, 0, 0};
    for (auto call : calls)
    {
        //bgfx::setTransform(call.transformMatrix);

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

        bgfx::setTransform(glm::value_ptr(call.transformMatrix));
        if (call.animationMatrices.size() > 0)
        {
            //bgfx::setUniform(animHandle, call.animationMatrices);

            var matrixCount = call.animationMatrices.size() + 1;

            std::vector<float> matrixData;
            matrixData.reserve(matrixCount * 16);

            {
                const float* transformPtr = glm::value_ptr(call.transformMatrix);
                matrixData.insert(matrixData.end(), transformPtr, transformPtr + 16);
            }

            for (const auto& full_vec : call.animationMatrices)
            {
                const float* matPtr = glm::value_ptr(full_vec);
                matrixData.insert(matrixData.end(), matPtr, matPtr + 16);
            }


            //bgfx::setTransform(glm::value_ptr(fullVec[0]));
            bgfx::setTransform(matrixData.data(), static_cast<uint16_t>(matrixCount));

        }

        lightUniforms->Apply(lights);

        call.mesh->use();

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
            dde.setTransform(glm::value_ptr(d.matrix));
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
    debugFuncs.clear();
    lights.clear();

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

void bgfx::setUniform(bgfx::UniformHandle handle, glm::vec4 v)
{
    bgfx::setUniform(handle, tmt::math::vec4toArray(v));
}

void bgfx::setUniform(bgfx::UniformHandle handle, std::vector<glm::vec4> v)
{
    float* arr = new float[v.size() * 4];
    for (int i = 0; i < v.size()*4; i+=4)
    {
        var vec = v[i/4];
        for (int j = 0; j < 4; ++j)
        {
            arr[i + j] = vec[j];
        }
    }

    bgfx::setUniform(handle, arr, v.size());

    delete[] arr;
}

void bgfx::setUniform(bgfx::UniformHandle handle, std::vector<glm::mat4> v)
{
    var arr = tmt::math::mat4ArrayToArray(v);
    bgfx::setUniform(handle, arr, v.size());

    delete[] arr;
}
