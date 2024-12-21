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

void tmt::render::Mesh::draw(glm::mat4 transform, Material *material, std::vector<glm::mat4> anims)
{
    var drawCall = DrawCall();

    drawCall.mesh = this;
    drawCall.state = material->GetMaterialState();
    drawCall.matrixMode = material->state.matrixMode;

    //drawCall.transformMatrices = std::vector<MatrixArray>(MAX_BONE_MATRICES + 1);

    {
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                drawCall.transformMatrix[x][y] = transform[x][y];
            }
        }
    }

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

tmt::render::SceneDescription::Node::Node(fs::BinaryReader* reader, SceneDescription* scene)
{
    name = reader->ReadString();

    position = reader->ReadVec3();

    var rot = reader->ReadVec4();
    rotation = glm::quat(rot.w, rot.x,rot.y,rot.z);

    scale = reader->ReadVec3();
    this->scene = scene;

    var meshCount = reader->ReadInt32();

    if (meshCount > 0)
    {
        var meshes = reader->ReadArray<int>(meshCount);
        for (int i = 0; i < meshCount; ++i)
        {
            meshIndices.push_back(meshes[i]);
        }
    }

    isBone = reader->Read<bool>();

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
        children.push_back(new Node(node->mChildren[i], scene));
    }
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

tmt::obj::Object* tmt::render::SceneDescription::Node::ToObject()
{
    var obj = new tmt::obj::Object();

    obj->name = name;
    obj->position = position;
    obj->rotation = glm::eulerAngles(rotation);
    obj->scale = scale;

    for (int mesh_index : meshIndices)
    {
        var mesh = scene->GetMesh(mesh_index);
        var mshObj = tmt::obj::MeshObject::FromBasicMesh(mesh);
        mshObj->material = mesh->model->CreateMaterial(mesh->model->materialIndices[mesh_index], defaultShader);
        mshObj->SetParent(obj);
    }

    for (auto child : children)
    {
        bool isArmature = (child->isBone && !isBone) || (child->name == "Armature");
        if (isArmature)
        {
            var skele = new SkeletonObject();
            skele->Load(child);
            obj->AddChild(skele);
            continue;
        }

        obj->AddChild(child->ToObject());
    }

    return obj;
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
            models.push_back(new Model(reader));
        }

        reader->close();
        delete reader;
    }
    else
    {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                   aiProcess_FindInvalidData | aiProcess_PreTransformVertices | aiProcess_PopulateArmatureData);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        name = scene->mRootNode->mName.C_Str();
        rootNode = new Node(scene->mRootNode, this);

        {
            var model = new Model(scene);
            models.push_back(model);
        }

        import.FreeScene();
    }
}

tmt::obj::Object* tmt::render::SceneDescription::ToObject()
{
    var obj = new tmt::obj::Object();
    obj->name = name;

    obj->AddChild(rootNode->ToObject());

    return obj;
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
            bone->rotation = glm::eulerAngles(child->rotation);
            bone->scale = child->scale;
            count++;
            bone->id = count;
            count = bone->Load(child, count);
            bone->SetParent(this);
        }
    }

    return count;
}

void tmt::render::SkeletonObject::Load(SceneDescription::Node* node)
{
    var ct = -1;
    for (auto child : node->children)
    {
        if (child->isBone)
        {
            var bone = new BoneObject();
            bone->name = child->name;
            bone->position = child->position;
            bone->rotation = glm::eulerAngles(child->rotation);
            bone->scale = child->scale;
            ct++;
            bone->id = ct;
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

std::tuple<glm::vec3, glm::quat, glm::vec3> tmt::render::Animator::AnimationBone::Update(float animationTime)
{
    var translation = InterpolatePosition(animationTime);
    var rotation = InterpolateRotation(animationTime);
    var scale = InterpolateScaling(animationTime);
    return std::make_tuple(translation, rotation, scale);
}

int tmt::render::Animator::AnimationBone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < channel->positions.size() - 1; ++index)
    {
        if (animationTime < channel->positions[index + 1].time)
            return index;
    }
    assert(0);
}

int tmt::render::Animator::AnimationBone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < channel->rotations.size() - 1; ++index)
    {
        if (animationTime < channel->rotations[index + 1].time)
            return index;
    }
    assert(0);
}

int tmt::render::Animator::AnimationBone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < channel->scales.size() - 1; ++index)
    {
        if (animationTime < channel->scales[index + 1].time)
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

glm::vec3 tmt::render::Animator::AnimationBone::InterpolatePosition(float animationTime)
{
    if (1 == channel->positions.size())
        return channel->positions[0].value;

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->positions[p0Index].time, channel->positions[p1Index].time, animationTime);
    glm::vec3 finalPosition =
        glm::mix(channel->positions[p0Index].value, channel->positions[p1Index].value, scaleFactor);
    return finalPosition;
}

glm::quat tmt::render::Animator::AnimationBone::InterpolateRotation(float animationTime)
{
    if (1 == channel->rotations.size())
    {
        auto rotation = glm::normalize(channel->rotations[0].value);
        return rotation;
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->rotations[p0Index].time, channel->rotations[p1Index].time, animationTime);
    glm::quat finalRotation =
        glm::slerp(channel->rotations[p0Index].value, channel->rotations[p1Index].value, scaleFactor);
    //finalRotation = glm::normalize(finalRotation);
    return finalRotation;
}

glm::vec3 tmt::render::Animator::AnimationBone::InterpolateScaling(float animationTime)
{
    if (1 == channel->scales.size())
        return channel->scales[0].value;

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(channel->scales[p0Index].time, channel->scales[p1Index].time, animationTime);
    glm::vec3 finalScale = glm::mix(channel->scales[p0Index].value, channel->scales[p1Index].value, scaleFactor);
    return finalScale;
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

        if (animationBones.size() <= currentAnimation->nodeChannels.size())
            LoadAnimationBones();

        for (auto animation_bone : animationBones)
        {
            var bone = skeleton->GetBone(animation_bone->channel->name);
            glm::vec3 pos, scl;
            glm::quat rot;
            std::tie(pos,rot,scl) = animation_bone->Update(time);

            debug::Gizmos::DrawSphere(pos, 0.1f);

            bone->position = pos;
            bone->rotation = glm::degrees(glm::eulerAngles(rot));
            bone->scale = scl;
        }
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

        animationBones.push_back(animBone);
    }
}

void tmt::render::SkeletonObject::Update()
{

    boneMatrices.clear();

    for (auto bone : bones)
    {
        var mat = bone->GetTransform();

        boneMatrices.push_back(mat);

        debug::Gizmos::DrawSphere(bone->GetGlobalPosition(), 0.1f);
    }

    Object::Update();
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
                                                   aiProcess_FindInvalidData | aiProcess_PreTransformVertices | aiProcess_PopulateArmatureData );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }

        LoadFromAiScene(scene);
    }
}

tmt::render::Model::Model(const aiScene *scene)
{ LoadFromAiScene(scene); }

void tmt::render::Model::LoadFromAiScene(const aiScene* scene)
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

        Vertex* verts = new Vertex[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), verts);

        u16* incs = new u16[indices.size()];
        std::copy(indices.begin(), indices.end(), incs);

        var mesh = createMesh(verts, incs, vertices.size(), indices.size(), Vertex::getVertexLayout(), this);

        meshes.push_back(mesh);
        materialIndices.push_back(msh->mMaterialIndex);
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
                nodeChannel->positions.push_back(
                    Animation::NodeChannel::NodeKey<glm::vec3>(posKey.mTime, math::convertVec3(posKey.mValue)));
            }

            for (int i = 0; i < channel->mNumScalingKeys; ++i)
            {
                var posKey = channel->mScalingKeys[i];
                nodeChannel->scales.push_back(
                    Animation::NodeChannel::NodeKey<glm::vec3>(posKey.mTime, math::convertVec3(posKey.mValue)));
            }

            for (int i = 0; i < channel->mNumRotationKeys; ++i)
            {
                var posKey = channel->mRotationKeys[i];
                nodeChannel->rotations.push_back(
                    Animation::NodeChannel::NodeKey<glm::quat>(posKey.mTime, math::convertQuat(posKey.mValue)));
            }

            animation->nodeChannels.push_back(nodeChannel);
        }

        animations.push_back(animation);
    }

}

tmt::render::Model::Model(fs::BinaryReader* reader)
{
    var tmdlSig = reader->ReadString(4);

    var scale = reader->ReadSingle();

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
            var key = NodeChannel::NodeKey<glm::vec3>();
            key.time = reader->ReadSingle();
            key.value = reader->ReadVec3();
            nodeChannel->positions.push_back(key);
        }

        var rct = reader->ReadInt32();
        for (int j = 0; j < rct; ++j)
        {
            var key = NodeChannel::NodeKey<glm::quat>();
            key.time = reader->ReadSingle();
            key.value = reader->ReadQuat();
            nodeChannel->rotations.push_back(key);
        }

        var sct = reader->ReadInt32();
        for (int j = 0; j < sct; ++j)
        {
            var key = NodeChannel::NodeKey<glm::vec3>();
            key.time = reader->ReadSingle();
            key.value = reader->ReadVec3();
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
        bone->id = id;
        bone->position = pos;
        bone->rotation = rot;
        bone->scale = scl;


        bones.push_back(bone);
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

tmt::render::Texture::Texture(int width, int height, bgfx::TextureFormat::Enum tf, u64 flags, const bgfx::Memory *mem, string name)
{
    handle = createTexture2D(width, height, false, 1, tf, flags, mem);

    bgfx::setName(handle, name.c_str());

    format = tf;
    this->width = width;
    this->height = height;
    this->name = name;
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
        .add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Int16)
        .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)
    .end();

    return layout;
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
        animHandle = createUniform("iu_boneMatrices", bgfx::UniformType::Mat4, MAX_BONE_MATRICES);

        subHandlesLoaded = true;
    }

    float t[4] = {static_cast<float>(counterTime), static_cast<float>(glm::sin(counterTime)),
                  static_cast<float>(glm::cos(counterTime)), 0};
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

        bgfx::setTransform(call.transformMatrix);
        bgfx::setUniform(animHandle, call.animationMatrices.data(), static_cast<u16>(call.animationMatrices.size()));

        setVertexBuffer(0, call.mesh->vbh, 0, call.mesh->vertexCount);
        setIndexBuffer(call.mesh->ibh, 0, call.mesh->indexCount);

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
