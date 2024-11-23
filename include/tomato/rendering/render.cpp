#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "engine.hpp"

#define PAR_SHAPES_IMPLEMENTATION
#include <set>
#include <stack>

#include "imgui_ext.hpp"
#include "par_shapes.h"

#include "misc/debug.h"

#include "stb/stb_image.h"
#include "util/filesystem_tm.h"

CLASS_DEFINITION(Component, tmCamera)
CLASS_DEFINITION(Component, tmRenderer)
CLASS_DEFINITION(Component, tmSkeleton)

const char* defvert = R"(

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 WorldPosition;
out vec4 ScreenPosition;
out vec4 LightSpacePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightMatrices[100];

const int MAX_BONES = 252;
const int MAX_BONE_INFLUENCE_NOREN = 4;
uniform mat4 boneTransforms_NOREN[MAX_BONES];

void main()
{

    vec4 updatedPosition = vec4(0.0f);
    vec3 updatedNormal = vec3(0.0f);
    
    for(int i = 0 ; i < MAX_BONE_INFLUENCE_NOREN; i++)
    {
        // Current bone-weight pair is non-existing
        if(boneIds[i] == -1) 
            continue;

        // Ignore all bones over count MAX_BONES
        if(boneIds[i] >= MAX_BONES) 
        {
            updatedPosition = vec4(aPos,1.0);
            break;
        }

        mat4 boneTransform = mat4(1.0);
        
        mat4 ts = mat4(1.0);

        boneTransform = boneTransforms_NOREN[boneIds[i]];
        //col += boneTransform[boneIds[i]];

        // Set pos
        vec4 localPosition = boneTransform * vec4(aPos,1.0);
        updatedPosition += localPosition * weights[i];
        // Set normal
        vec3 localNormal = mat3(boneTransform) * aNormal;
        updatedNormal += localNormal * weights[i];
    }

    if (updatedPosition == vec4(0)) {
        //updatedPosition = vec4(aPos,1.0);
    }

    if (boneIds == ivec4(-1)) {
        updatedPosition = vec4(aPos,1.0);
    }

    if (updatedNormal == vec3(0)) {
        updatedNormal = aNormal;
    }

    TexCoords = aTexCoords;    
    WorldPosition = vec3(model * updatedPosition);
    ScreenPosition = projection * view * vec4(WorldPosition,1.0);
    Normal = mat3(transpose(inverse(model))) * updatedNormal;
	LightSpacePosition = lightMatrices[0] * vec4(WorldPosition,1.0);

    gl_Position =  ScreenPosition;
}

)";

const char* deffrag = R"(

#version 330 core
layout (location = 0) out vec4 gAlbedoSpec;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition;
layout (location = 3) out vec4 gShading;
layout (location = 4) out vec4 gLightPosition;

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPosition;
in vec4 ScreenPosition;
in vec4 LightSpacePosition;

uniform sampler2D albedo;
uniform sampler2D roughness;
uniform sampler2D metallic;
uniform sampler2D normal;
uniform sampler2D color_map;
uniform bool do_multiply;

uniform float actorIndex_NOREN;
uniform float actorCount_NOREN;

uniform float metallic_override;
uniform float roughness_override;

uniform vec3 color_col;

uniform bool use_color;
uniform bool use_colormap;
uniform bool use_roughmap;
uniform bool use_metmap;
uniform bool use_normap;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPosition);
    vec3 Q2  = dFdy(WorldPosition);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    gAlbedoSpec = texture(albedo, TexCoords);
    gPosition = (WorldPosition);
    gNormal = normalize(Normal);
    gShading = vec4(1,texture(roughness,TexCoords).r,texture(metallic,TexCoords).r,1);
	gLightPosition = LightSpacePosition;
	//gLightPosition = vec4(1);
    //gAlbedoSpec = WorldPosition;

    if (use_color)
        gAlbedoSpec = vec4(color_col,1.0);
    if (use_colormap) {
		if (do_multiply)
			gAlbedoSpec *= (vec4(color_col,1) * texture(color_map, TexCoords));
		else
			gAlbedoSpec = mix(gAlbedoSpec,vec4(color_col,1),texture(color_map, TexCoords).r);
	}
    if (!use_roughmap)
        gShading.g = roughness_override;
    if (!use_metmap)
        gShading.b = metallic_override;

    gShading.r = (actorIndex_NOREN) / (actorCount_NOREN);

    if (use_normap)
        gNormal = getNormalFromMap();

    //FragColor = vec4(Normal, 1.0);

}

)";








tmBone::~tmBone()
{
    m_Positions.clear();
    m_Rotations.clear();
    m_Scales.clear();
}

tmAnimation::tmAnimation(const std::string& animationPath, tmModel* model)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);

	for (int i = 0; i < scene->mNumAnimations; ++i)
	{
        auto animation = scene->mAnimations[i];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        name = animation->mName.C_Str();

        if (name.empty())
        {
            name = fs::path(animationPath).filename().string()+"_"+std::to_string(i);
        }

        ReadHeirarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);

        animationIndex.Add(this);
	}
}

tmAnimation::~tmAnimation()
{
    m_Bones.clear();
}

CLASS_DEFINITION(Component, tmAnimator)

std::shared_ptr<Component> tmAnimator::clone(int offset)
{
    var sc = std::make_shared<tmAnimator>(*this);
    sc->skeletonIndex += offset;

    return sc;
}

CLASS_DEFINITION(tmRenderer, tmMeshRenderer)
CLASS_DEFINITION(tmMeshRenderer, tmSkinnedMeshRenderer)

std::shared_ptr<Component> tmSkinnedMeshRenderer::clone(int offset)
{
    var sc = std::make_shared<tmSkinnedMeshRenderer>(*this);

    if (this->bones.GetCount() > 0) {


        for (int i = 0; i < bones.GetCount(); ++i)
        {
            sc->bones[i] = bones[i] + offset;
        }
    }

    return sc;
}

tmSkinnedMeshRenderer::tmSkinnedMeshRenderer(tmModel* model, int meshIndex)
{
    Initialize(model, meshIndex);
}

Dictionary<string, tmModel*> tmModel::loadedModels;
tmCamera* tmCamera::_mainCamera;
List<tmShader*> tmShader::shader_index;

tmMeshRenderer::tmMeshRenderer(tmModel* model, int meshIndex)
{
    Initialize(model, meshIndex);
}

void tmMeshRenderer::Deserialize(nlohmann::json j)
{
	tmRenderer::Deserialize(j);

    Initialize(tmModel::LoadModel(j["model_path"]), j["mesh_index"]);

    materialIndex = j["material_index"];
}

void tmMeshRenderer::EngineRender()
{
	tmRenderer::EngineRender();

    var material = tmeGetCore()->renderMgr->materials[materialIndex];

    ImGui::Text(("Material Index: " + std::to_string(materialIndex)).c_str());

    {

        static string file_path = "";
        static bool found = false;
        static int actorN;

        if (ImGui::Button("Select.."))
            ImGui::OpenPopup("mat_popup");
        ImGui::SameLine();
        ImGui::TextUnformatted(materialIndex == -1 ? "<None>" : material->name.c_str());
        if (ImGui::BeginPopup("mat_popup"))
        {
            ImGui::SeparatorText("Material");
            for (int i = 0; i < tmeGetCore()->renderMgr->materials.size(); i++)
                if (ImGui::Selectable(tmeGetCore()->renderMgr->materials[i]->name.c_str()))
                    materialIndex = i;

            ImGui::Separator();

            if (ImGui::Selectable("Add new Material")) {

                var mat = new tmMaterial(material->shader);

                materialIndex = mat->materialIndex;

                material = mat;
            }
            ImGui::EndPopup();
        }
    }

    material->EngineRender();

    /*
	size_t i1 = mesh->vertexCount;
	for (int i = 0; i < i1; ++i)
	{
        if (i > 0) {
            var vert = mesh->verts[i];

            int nextI = i + 1;
            if (nextI >= i1)
            {
                nextI = 0;
            }

            var nextVert = mesh->verts[nextI];

            vec3 startPoint = (vert.position);
            vec3 endPoint =(nextVert.position);

            startPoint *= transform()->GetGlobalScale();
            endPoint *= transform()->GetGlobalScale();

            startPoint += transform()->GetGlobalPosition();
            endPoint += transform()->GetGlobalPosition();

            startPoint = transform()->TransformPoint(startPoint);
            endPoint = transform()->TransformPoint(endPoint);

            tmglDebugRenderer::line(startPoint, endPoint, dd::colors::Orange);
  
        }

	}
	*/

}

tmShader::tmShader(string vertex, string fragment, bool isPath)
{
    vertData = vertex;
    fragData = fragment;
    this->isPath = isPath;
    shader_index.Add(this);

    reload(vertex,fragment,isPath);
}

tmShader::~tmShader()
{
	unload();
}

void tmShader::reload(string vertex, string fragment, bool isPath)
{
	unload();
    id = -1;


    bool shader_found = false;
    u32 _shaderId = -1;

    /*
    for (auto shader : shader_index.GetVector())
    {
	    if (shader->vertData == vertex)
	    {
		    if (shader->fragData == fragment)
		    {
			    if (shader->isPath == isPath)
			    {
                    shader_found = true;
                    _shaderId = shader->id;
                    break;
			    }
		    }
	    }
    }
    */

    if (!isPath) {
        id = tmgl::genShader(vertex.c_str(), fragment.c_str());
    }
    else {
        string vsrc = tmfs::loadFileString(vertex);
        string fsrc = tmfs::loadFileString(fragment);

        var nid = tmgl::genShader(vsrc.c_str(), fsrc.c_str());

        this->id = nid;

        std::cout << this->id << std::endl;

    }

}

static unsigned current_shader_id = -1;

void tmShader::use()
{
    if (current_shader_id != id) {
        glUseProgram(id);
        current_shader_id = id;
    }
}

tmCamera::tmCamera()
{
    _mainCamera = this;
}

void tmCamera::Deserialize(nlohmann::json j)
{
	Component::Deserialize(j);

    FieldOfView = j["fov"];
    Size = j["size"];
    NearPlane = j["near_plane"];
    FarPlane = j["far_plane"];
    EnableOcclusionCulling = j["occlusion_culling"];
    ClearFlags = j["clear_flags"];
    Projection = j["projection"];
    glm::from_json(j["clear_color"], ClearColor);
    framebuffer = new tmFramebuffer(j["framebuffer_type"], {tmeGetCore()->ScreenWidth, tmeGetCore()->ScreenHeight});
}

void tmCamera::Start()
{
	//Logger::logger << "Camera initialized";
}

void tmCamera::Update()
{

    // real updating
    var actor = GetActor();
   var yaw = actor->transform->rotation.y;
    var pitch = actor->transform->rotation.x;
    var cameraFront = transform()->GetForward();

    //cameraFront = actor->transform->GetForward();
    //cameraFront = { 0,0,-1 };

    var pos = actor->transform->position;

    //pos = { 0,0,3 };

    view = glm::lookAt(pos, pos+cameraFront, actor->transform->GetUp());

    var engine = tmeGetCore();

    float aspect = engine->ScreenWidth / engine->ScreenHeight;
    proj = glm::perspective(glm::radians(FieldOfView), aspect, NearPlane, FarPlane);
    orthoProj = glm::ortho(0.0f, engine->ScreenWidth, 0.0f, engine->ScreenHeight, -1.0f, 1.0f);

    switch (Projection)
    {
    case Perspective:
        break;
    case Orthographic:
        //proj = glm::ortho
        break;
    }



}

void tmCamera::LateUpdate()
{
    if (tmeGetCore()->runtimePlaying && framebuffer) {
        framebuffer->cameraPosition = GetActor()->transform->position;
        // drawing
        Use();

        tmeGetCore()->lighting->skybox->draw(this);

        for (auto material : tmeGetCore()->renderMgr->materials)
            UpdateShader(material->shader, material->settings.useOrtho);

        var renderMgr = tmeGetCore()->renderMgr;
        renderMgr->Draw(this);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void tmBaseCamera::Use()
{
    if (framebuffer != nullptr)
    {
        framebuffer->clearColor = ClearColor;
        framebuffer->use();
    }
}

nlohmann::json tmCamera::Serialize()
{
	var c = Component::Serialize();

    glm::to_json(c["clear_color"], ClearColor);

    c["fov"] = FieldOfView;
    c["size"] = Size;
    c["near_plane"] = NearPlane;
    c["far_plane"] = FarPlane;
    c["occlusion_culling"] = EnableOcclusionCulling;

    c["clear_flags"] = ClearFlags;
    c["projection"] = Projection;
    c["framebuffer_type"] = framebuffer->framebuffer_type;

    return c;
}

void tmCamera::EngineRender()
{
    glm::vector_color("Clear Color", ClearColor);

    ImGui::DragFloat("Field of View", &FieldOfView, 1, 0);
    ImGui::DragFloat("Near Plane", &NearPlane, 1, EPSILON);
    ImGui::DragFloat("Far Plane", &FarPlane, 1, NearPlane);

    ImGui::Checkbox("Enable occlusion culling", &EnableOcclusionCulling);

    float scale = ImGui::GetWindowSize().x / framebuffer->size.x;

    ImGui::Image((ImTextureID)(framebuffer->gAlb), ImVec2(framebuffer->size.x*scale, framebuffer->size.y*scale), ImVec2(0, 1), ImVec2(1, 0));

    mat4 clip = glm::inverse(GetProjectionMatrix() * GetViewMatrix());
    tmglDebugRenderer::frustum(clip, dd::colors::White);
}

tmCamera* tmCamera::GetMainCamera()
{
    return _mainCamera;
}

tmShader* defaultShader;

tmShader* tmShader::GetDefaultShader()
{
	if (!defaultShader)
	{
        defaultShader = new tmShader(defvert, deffrag, false);
	}

    return defaultShader;
}

void tmBaseCamera::UpdateShader(tmShader* shader, bool orthoProj)
{
    shader->setMat4("view", view);
    if (orthoProj)
        shader->setMat4("projection", this->orthoProj);
    else
		shader->setMat4("projection", proj);

    tmeGetCore()->lighting->Update(shader);
}


tmVertexBuffer::tmVertexBuffer(tmVertex* data,size_t count, size_t elementCount, unsigned int* indices)
{
    unsigned vao = tmgl::genVertexArray();
    unsigned vbo = tmgl::genBuffer(GL_ARRAY_BUFFER, &data[0],  count * sizeof(tmVertex), GL_STATIC_DRAW);
    if (indices != nullptr) EBO = tmgl::genBuffer(GL_ELEMENT_ARRAY_BUFFER, &indices[0], elementCount * sizeof(unsigned int), GL_STATIC_DRAW);
    tmgl::genVertexBuffer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex));
    tmgl::genVertexBuffer(1, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,normal));
    tmgl::genVertexBuffer(2, 2, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,texcoords));
    tmgl::genVertexBuffer(3, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,m_BoneIDs));
    tmgl::genVertexBuffer(4, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,m_Weights));
    glBindVertexArray(0);

    VAO = vao;
    VBO = vbo;
    vertexCount = count;
    this->elementCount = elementCount;
    shader = 0;
}

tmVertexBuffer::~tmVertexBuffer()
{
    tmgl::freeBuffer(VBO);
    tmgl::freeBuffer(EBO);
}


tmMaterial::tmShaderField tmMaterial::GetField(string name)
{
    for (auto field : fields.GetVector())
        if (field.name == name) return field;

    return tmShaderField();
}

void tmMaterial::SetField(string name, tmShaderField field)
{
    int i = 0;
    for (auto shader_field : fields.GetVector()) {
        if (shader_field.name == name) break;
        i++;
    }

    if (i >= fields.GetCount())
        return;

    fields[i] = field;

}

fieldtypedecl(int)
fieldtypedecl(float)
fieldtypedecl(bool)

fieldtypedecl(vec2)
fieldtypedecl(vec3)
fieldtypedecl(vec4)

fieldtypedecl(mat2)
fieldtypedecl(mat3)
fieldtypedecl(mat4)

void tmMaterial::SetTexture(string name, tmTexture* texture)
{

    var field = GetField(name);

    if (field.type != NULLTYPE)
    {

        field.data.int_0 = texture->engineId;

    }

    SetField(name, field);

}

tmMaterial::tmMaterial(tmShader* shader)
{
    materialIndex = tmeGetCore()->renderMgr->materials.size();
    name = "New Material " + std::to_string(materialIndex);
    tmeGetCore()->renderMgr->materials.push_back(this);
    this->shader = shader;

    Reload();
}

void tmMaterial::Reload()
{
    shader->reload(shader->vertData,shader->fragData,shader->isPath);
    GLint i;
    GLint count;

    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 16 * 4; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length
    glGetProgramiv(shader->id, GL_ACTIVE_UNIFORMS, &count);
    //printf("Active Uniforms: %d\n", count);

    for (i = 0; i < count; ++i)
    {
        glGetActiveUniform(shader->id, (GLuint)i, bufSize, &length, &size, &type, name);

        //Logger::logger << name << std::endl;
        //Logger::logger << std::to_string(type) << std::endl;

        var field = tmShaderField();
        field.name = name;
        field.type = (tmFieldType)type;
        field.data.int_0 = 0;
        field.data.float_0 = 0;
        field.data.bool_0 = false;

        field.data.vec2_0 = vec2(0);
        field.data.vec3_0 = vec3(0);
        field.data.vec4_0 = vec4(0);

        field.data.mat2_0 = mat2(0);
        field.data.mat3_0 = mat3(0);
        field.data.mat4_0 = mat4(0);

        if (GetField(field.name).type == NULLTYPE) {
            fields.Add(field);
        }
    }
}

void tmMaterial::EngineRender()
{
    if (ImGui::CollapsingHeader("Material"))
    {

        var material = this;

        for (auto field : material->fields.GetVector())
        {

            string name = field.name;

            if (StringContains(name, "_NOREN"))
                continue;
            bool isColor = StringContains(name, "_col");

            switch (field.type)
            {
            case tmMaterial::INT:
                ImGui::DragInt(name.c_str(), &field.data.int_0);
                break;
            case tmMaterial::FLOAT:
                ImGui::DragFloat(name.c_str(), &field.data.float_0, 0.1f);
                break;
            case tmMaterial::BOOL:
                ImGui::Checkbox(name.c_str(), &field.data.bool_0);
                break;
            case tmMaterial::VEC2:

                glm::vector_drag(name.c_str(), field.data.vec2_0);

                break;
            case tmMaterial::VEC3:

                if (isColor)
                {
                    glm::vector_color(name.c_str(), field.data.vec3_0);
                }
                else
                {
                    glm::vector_drag(name.c_str(), field.data.vec3_0);
                }

                break;
            case tmMaterial::VEC4:

                if (isColor)
                {
                    glm::vector_color(name.c_str(), field.data.vec4_0);
                }
                else
                {
                    glm::vector_drag(name.c_str(), field.data.vec4_0);
                }

                break;

            case tmMaterial::SAMPLER2D:
            {

                var tex = GetTexture(field.data.int_0);

                tex = ImGui::TexturePicker(name.c_str(), tex);

                field.data.int_0 = tex->engineId;

            }
            break;
            case tmMaterial::SAMPLERCUBE:
                break;

            }

            material->SetField(name, field);

        }
    }
}

tmMesh::tmMesh(tmVertex* verts, size_t vertCount, size_t elemCount, unsigned* ind)
{
    vertices = verts;
    indices = ind;
    vertexCount = vertCount;
    elementCount = elemCount;

    setup();

}

tmMesh::~tmMesh()
{
    delete[] vertices;
}

tmDrawCall* tmMesh::draw(unsigned material, glm::mat4 modelMatrix, tmActor* actor, std::vector<mat4> boneTransforms)
{
    return tmeGetCore()->renderMgr->InsertCall(&buffer, material, boneTransforms, modelMatrix,actor);
}


void tmMesh::setup()
{

    buffer = tmVertexBuffer(vertices, vertexCount,elementCount, indices);
}

nlohmann::json tmMeshRenderer::Serialize()
{
    var c = tmRenderer::Serialize();

    c["mesh_index"] = meshIndex;
    c["material_index"] = materialIndex;
    c["model_path"] = model->path;

    return c;

}

void tmMeshRenderer::Update()
{

    var material = tmeGetCore()->renderMgr->materials[materialIndex];

    std::vector<mat4> boneMatrix;

    for (int i = 0; i < MAX_BONES; ++i)
    {
        boneMatrix.push_back(mat4(1.0));
    }

    var drawCall = mesh->draw(materialIndex, GetActor()->transform->GetMatrix(), actor(), boneMatrix);
}

void tmMeshRenderer::Initialize(tmModel* model, int meshIndex)
{
    mesh = model->meshes[meshIndex];
    this->meshIndex = meshIndex;
    this->model = model;
}

void CalculateBoneTransform(int rootIndex, glm::mat4 parentTransform, List<glm::mat4>& m_FinalBoneMatrices, std::map<string, tmBoneInfo> boneInfoMap)
{
    struct Node {
        int index;
        glm::mat4 parentTransform;
    };

    std::stack<Node> stack;
    stack.push({ rootIndex, parentTransform });

    while (!stack.empty())
    {
        Node current = stack.top();
        stack.pop();

        auto actor = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(current.index);
        auto transform = actor->transform;

        const glm::mat4& nodeTransform = transform->GetLocalMatrix();
        glm::mat4 globalTransformation = glm::mulmatSIMD(current.parentTransform, nodeTransform);

        const std::string& nodeName = actor->name;
        auto it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end())
        {
            int boneIndex = it->second.id;
            const glm::mat4& offset = it->second.offset;
            m_FinalBoneMatrices[boneIndex] = glm::mulmatSIMD(globalTransformation, offset);
        }

        auto& children = transform->GetChildren();
        int childCount = children.GetCount();
        for (int i = 0; i < childCount; ++i)
        {
            int childIndex = children[i];
            stack.push({ childIndex, globalTransformation });
        }
    }
}


void tmSkinnedMeshRenderer::Start()
{
}

void tmSkeleton::Start()
{
    std::vector<mat4> boneMatrix;

    for (int i = 0; i < MAX_BONES; ++i)
    {
        boneMatrix.push_back(mat4(1.0));
    }

    bone_transforms = boneMatrix;
}

void tmSkeleton::Update()
{
    CalculateBoneTransform(armatureIndex, mat4(1.0), bone_transforms, m_BoneInfoMap);
}

List<mat4> tmSkeleton::GetBoneTransforms()
{
    return bone_transforms;
}

void tmSkinnedMeshRenderer::Update()
{

    std::vector<mat4> boneMatrix;

    for (int i = 0; i < MAX_BONES; ++i)
    {
        boneMatrix.push_back(mat4(1.0));
    }

    int i = 0;

    var skeleton = actor()->GetComponentInParent<tmSkeleton>();

    if (skeleton)
    {
        skeleton->m_BoneInfoMap = mesh->model->m_BoneInfoMap;
        boneMatrix = skeleton->GetBoneTransforms().GetVector();
    }

    var material = tmeGetCore()->renderMgr->materials[materialIndex];

    var drawCall = mesh->draw(materialIndex, GetActor()->transform->GetMatrix(), actor(), boneMatrix);
}

tmModel* tmModel::LoadModel(string path, tmModelSettings settings)
{

    if (loadedModels.Contains(path)) {
        return loadedModels[path];
    }

    return new tmModel(path, settings);
}

tmModel* tmModel::GenerateModel(PrimitiveType type)
{
    var model = new tmModel;

    par_shapes_mesh* parMesh = nullptr;

    switch (type)
    {
    case CUBE:
        parMesh = par_shapes_create_cube();
        break;
    case SPHERE:
        parMesh = par_shapes_create_subdivided_sphere(4);
        break;
    }

    std::vector<tmVertex> vertices;
    std::vector<unsigned> indices;

    for (int i = 0; i < parMesh->npoints*3; i += 3)
    {
        tmVertex vertex = {};

        vertex.position[0] = parMesh->points[i];
        vertex.position[1] = parMesh->points[i + 1];
        vertex.position[2] = parMesh->points[i + 2];

        if (parMesh->normals) {
            vertex.normal[0] = parMesh->normals[i];
            vertex.normal[1] = parMesh->normals[i + 1];
            vertex.normal[2] = parMesh->normals[i + 2];
        }

        if (parMesh->tcoords) {
            //vertex.texcoords[0] = parMesh->points[i];
            //vertex.texcoords[1] = parMesh->points[i + 1];
        }

        vertices.push_back(vertex);
    }

    for (int i = 0; i < parMesh->ntriangles; ++i)
    {
        indices.push_back(parMesh->triangles[i]);
    }

    var mesh = new tmMesh(vertices.data(), vertices.size(), indices.size(), indices.data());

    model->meshes.Add(mesh);

    par_shapes_free_mesh(parMesh);

    return model;
}

tmModel::~tmModel()
{
    for (auto mesh : meshes)
        delete mesh;
    m_BoneInfoMap.clear();

    delete prefab;
}

tmModel::tmModel(string path, tmModelSettings settings)
{
    this->settings = settings;
    loadModel(path);
}

tmModel::tmModel()
{
    this->path = "UNAVAILABLE";
}

void tmModel::loadModel(string path)
{

    this->path = path;
    prefab = new tmPrefab;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ForceGenNormals |  aiProcess_GenSmoothNormals | aiProcess_PopulateArmatureData);

    if (!scene || !scene->mRootNode)
    {
	    Logger::logger << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    scene_ = const_cast<aiScene*>(scene);

    //directory = path.substr(0, path.find_last_of('/'));
    int ct = tmeGetCore()->GetActiveScene()->actorMgr->GetActorCount();

    List<tmActor*> bones;

    processNode(scene->mRootNode, scene, nullptr,bones, false, scene->mName.C_Str());

    ct = tmeGetCore()->GetActiveScene()->actorMgr->GetActorCount() - ct;

    for (int i = 0; i < prefab->actors.GetCount(); ++i)
    {
        var actor = prefab->actors[i];

        var skinned = actor->GetComponent<tmSkinnedMeshRenderer>();
        if (skinned) {
            for (auto bone : bones.GetVector())
            {
                skinned->bones.Add(bone->id);
            }
        }

    }


    var animator = prefab->actors[0]->AttachComponent<tmAnimator>();
    if (prefab->actors[0]->transform->GetChildContains("Armature", prefab->actors.GetVector())) {
        animator->skeletonIndex = prefab->actors[0]->transform->GetChildContains("Armature", prefab->actors.GetVector())->entityId;
    }

    if (scene->HasAnimations()) {
        var anims = List<tmAnimation*>();

        for (int i = 0; i < scene->mNumAnimations; ++i)
        {
            var animation = new tmAnimation(path, i, this);
            anims.Add(animation);
        }

        animator->currentAnimation = anims[0];
    }

    if (scene->HasMaterials())
    {
	    for (int i = 0; i < scene->mNumMaterials; ++i)
	    {
            var amat = scene->mMaterials[i];
            var mat = new tmMaterial(tmShader::GetDefaultShader());
            mat->name = amat->GetName().C_Str();
            for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++) {
                aiTextureType textureType = static_cast<aiTextureType>(type);
                unsigned int textureCount = amat->GetTextureCount(textureType);

                for (unsigned int i = 0; i < textureCount; i++) {
                    aiString p;

                    string modelPath = path;
                        std::replace(modelPath.begin(),modelPath.end(), '/', '\\');

                    std::string directory;
                    std::string realPath;
                    const std::size_t last_slash_idx = modelPath.rfind('\\');
                    if (std::string::npos != last_slash_idx)
                    {
                        directory = modelPath.substr(0, last_slash_idx);
                    }

                    if (amat->GetTexture(textureType, i, &p) == AI_SUCCESS) {

                        realPath = directory + "//" + (fs::path(p.C_Str()).filename().string());

                        if (!fs::exists(realPath)) {
	                        std::cout << "Texture does not exist at " << realPath << std::endl;
                            continue;
                        }

                        std::cout << "Texture type " << textureType << ": " << realPath << std::endl;


                        switch (textureType)
                        {
                        case aiTextureType_DIFFUSE:
                            mat->SetTexture("albedo", new tmTexture(realPath, false));
                            break;
                        case aiTextureType_SHININESS:
                        {
                        		mat->SetTexture("roughness", new tmTexture(realPath, false));
                                mat->SetField("use_roughmap", true);
                        }
                            break;
                        case aiTextureType_METALNESS:
                        {
                            mat->SetTexture("metallic", new tmTexture(realPath, false));
                            mat->SetField("use_metmap", true);
                        }
                            break;
                        case aiTextureType_NORMALS:
                        {
                            mat->SetTexture("normal", new tmTexture(realPath, false));
                            mat->SetField("use_normap", true);
                        }
                            break;
                        }

                    }
                }
            }
	    }
    }
}

// Function to check if an aiNode is a bone
bool IsNodeABone(const aiScene* scene, const aiNode* node) {

    std::set<std::string> boneNames;

    // Collect all bone names from all meshes
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
            aiBone* bone = mesh->mBones[j];
            boneNames.insert(bone->mName.C_Str());
        }
    }

    // Check if the node's name is in the set of bone names
    return boneNames.find(node->mName.C_Str()) != boneNames.end();
}

void tmModel::processNode(aiNode* node, const aiScene* scene, tmActor* parent, List<tmActor*>& bones, bool isBone, string overrideName)
{
    var actor = prefab->InsertActor(node->mName.C_Str());

    if (isBone || IsNodeABone(scene, node))
    {
        bones.Add(actor);
    }
    else
    {
    }

    if (StringContains(actor->name, "Armature"))
        isBone = true;

    aiMatrix4x4 scalingMatrix;
    aiMatrix4x4::Scaling(aiVector3D(settings.importScale), scalingMatrix);

    if (parent) {
        actor->transform->SetParent(parent->transform);
    } else
    {
        actor->transform->parentId = -1;
    }

    if (!overrideName.empty())
    {
        actor->name = overrideName;
    }

    {
        actor->transform->CopyTransforms(AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation * scalingMatrix));
    }

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.Add(processMesh(mesh, scene));
        meshes[meshes.GetCount() - 1]->model = this;

        var meshRenderer = actor->AttachComponent<tmSkinnedMeshRenderer>(this, meshes.GetCount()-1);
        meshRenderer->materialIndex = mesh->mMaterialIndex + (tmeGetCore()->renderMgr->materials.size());

        //std::cout << "Added mesh. " << mesh->mName.C_Str() << std::endl;
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, actor, bones, isBone);
    }
}

tmMesh* tmModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<tmVertex> vertices;
	std::vector<unsigned int> indices;

    aiMatrix4x4 scalingMatrix;
    aiMatrix4x4::Scaling(aiVector3D(settings.importScale), scalingMatrix);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        tmVertex vertex;
        // process vertex positions, normals and texture coordinates

        SetVertexBoneDataToDefault(vertex);

        glm::vec3 vector;
        auto m_vertex = mesh->mVertices[i];

        m_vertex *= scalingMatrix;

        vector.x = m_vertex.x;
        vector.y = m_vertex.y;
        vector.z = m_vertex.z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texcoords = vec;
        }
        else
            vertex.texcoords = glm::vec2(0.0f, 0.0f);

    	vertices.push_back(vertex);
    }
    ExtractBoneWeightForVertices(vertices, mesh, scene);

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    var tmesh = new tmMesh(vertices.data(), vertices.size(),indices.size(), indices.data());
    tmesh->verts = vertices;
    return tmesh;
}

void tmAnimator::Start()
{

    skeleton_ = actor()->GetComponent<tmSkeleton>();

    if (!skeleton_)
		skeleton_ = actor()->AttachComponent<tmSkeleton>();
}

void tmAnimator::Update()
{

    m_DeltaTime = tmEngine::tmTime::deltaTime;

    if (currentAnimation)
    {
        skeleton_->armatureIndex = skeletonIndex;
        m_CurrentTime += (currentAnimation->GetTicksPerSecond() * m_DeltaTime) * (60 / currentAnimation->GetTicksPerSecond());
        m_CurrentTime = fmod(m_CurrentTime, currentAnimation->GetDuration());
        m_CurrentTime = glm::clamp(m_CurrentTime,0.f, INFINITY);

        if (std::isnan(m_CurrentTime))
            m_CurrentTime = 0;

        var skeleton = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(skeletonIndex)->transform;

        for (auto childIndex : skeleton->GetRecursiveChildren())
        {
            var child = tmActorMgr::GetSceneActor(childIndex);

            tmBone* Bone = currentAnimation->FindBone(child->name);

            if (Bone)
            {
                Bone->Update(m_CurrentTime);
                child->transform->CopyTransforms(Bone->GetLocalTransform());
            }


        }

    }

}

List<tmAnimation*> tmAnimation::animationIndex;

void tmAnimator::EngineRender()
{
    {

        static string file_path = "";
        static bool found = false;
        static int actorN;

        if (ImGui::Button("Select.."))
            ImGui::OpenPopup("anim_popup");
        ImGui::SameLine();
        ImGui::TextUnformatted(currentAnimation == nullptr ? "<None>" : currentAnimation->name.c_str());
        if (ImGui::BeginPopup("anim_popup"))
        {
            ImGui::SeparatorText("Animation");
            for (int i = 0; i < tmAnimation::animationIndex.GetCount(); i++)
                if (ImGui::Selectable(tmAnimation::animationIndex[i]->name.c_str()))
                    currentAnimation = tmAnimation::animationIndex[i];
            ImGui::EndPopup();
        }
    }
}

void tmRenderMgr::DeserializeGame(nlohmann::json g)
{

    std::vector<tmTexture*> texs;
    std::vector<tmCubemap*> cubetexs;

    if (tmTexture::textures.GetCount() >= g["textures"].size()) {
    }
    else {
        for (auto t : g["textures"])
        {
            texs.push_back(new tmTexture(t["path"], t["flip"]));
        }
    }

    for (auto t : g["cube_textures"])
    {
        cubetexs.push_back(new tmCubemap(t["path"], t["flip"]));
    }

    for (int i = 0; i < g["materials"].size(); ++i)
    {

        var j = g["materials"][i];

        var shader = new tmShader(j["vertex"], j["fragment"], j["isPath"]);

        var mat = new tmMaterial(shader);

        if (!j["fields"].is_null()) {

            for (auto jfield : j["fields"])
            {
                var field = mat->GetField(jfield["name"]);

                if (field.type == tmMaterial::INT)
                {
                    var data_int = field.data.int_0;
                    glm::from_json(jfield["data"], data_int);
                    field.data.int_0 = data_int;
                }

                if (field.type == tmMaterial::FLOAT)
                {
                    var data_float = field.data.float_0;

                    glm::from_json(jfield["data"], data_float);
                    field.data.float_0 = data_float;
                }

                if (field.type == tmMaterial::BOOL)
                {
                    var data_bool = field.data.bool_0;
                    glm::from_json(jfield["data"], data_bool);
                    field.data.bool_0 = data_bool;
                }

                if (field.type == tmMaterial::VEC2)
                {
                    var data_vec2 = field.data.vec2_0;
                    glm::from_json(jfield["data"], data_vec2);
                    field.data.vec2_0 = data_vec2;
                }

                if (field.type == tmMaterial::VEC3)
                {
                    var data_vec3 = field.data.vec3_0;
                    glm::from_json(jfield["data"], data_vec3);
                    field.data.vec3_0 = data_vec3;
                }

                if (field.type == tmMaterial::VEC4)
                {
                    var data_vec4 = field.data.vec4_0;
                    glm::from_json(jfield["data"], data_vec4);
+                     field.data.vec4_0 = data_vec4;
                }

                if (field.type == tmMaterial::MAT2)
                {
                    var data_mat2 = field.data.mat2_0;
                    glm::from_json(jfield["data"], data_mat2);
                    field.data.mat2_0 = data_mat2;
                }

                if (field.type == tmMaterial::MAT3)
                {
                    var data_mat3 = field.data.mat3_0;
                    glm::from_json(jfield["data"], data_mat3);
                    field.data.mat3_0 = data_mat3;
                }

                if (field.type == tmMaterial::MAT4)
                {
                    var data_mat4 = field.data.mat4_0;
                    glm::from_json(jfield["data"], data_mat4);
                    field.data.mat4_0 = data_mat4;
                }

                if (field.type == tmMaterial::SAMPLER2D)
                {
                    int idx = jfield["data"];

                    std::cout << jfield["name"] << ":" << idx << std::endl;

                    var tex = tmTexture::GetTexture(idx);
                    field.data.int_0 = idx;
                }

                if (field.type == tmMaterial::SAMPLERCUBE)
                {
                    var cubetex = tmTexture::GetTexture(jfield["data"].get<int>());
                    field.data.int_0 = cubetex->engineId;
                }

                mat->SetField(field.name, field);

            }

        }
    }
}

nlohmann::json tmRenderMgr::SerializeGame()
{

    nlohmann::json j;

    List<tmTexture*> texs = List<tmTexture*>();
    List<tmTexture*> cubetexs = List<tmTexture*>();

    for (int i = 0; i < materials.size(); ++i)
    {
        var material = materials[i];

        nlohmann::json m;

        m["vertex"] = material->shader->vertData;
        m["fragment"] = material->shader->fragData;
        m["isPath"] = material->shader->isPath;

        m["fields"] = nlohmann::json();

        int fieldCount = material->fields.GetCount();

        for (int i = 0; i < fieldCount - 1; ++i)
        {

            if (i >= fieldCount)
                break;

            tmMaterial::tmShaderField field;
            field = material->fields[i];

            std::cout << i << std::endl;


            //std::cout << field.name << std::endl;
            //std::cout << fieldCount  << std::endl;
            //std::cout << i << std::endl;

            if (field.name == "model" || field.name == "projection" || field.name == "view") {
                if (i == fieldCount - 1)
                {
                    break;
                }
                else {
                    continue;
                }
            }

            var jfield = nlohmann::json();
            jfield["name"] = field.name;
            jfield["type"] = field.type;
            jfield["data"] = nlohmann::json(0);

            if (field.type == tmMaterial::SAMPLERCUBE)
            {
                var texcube = tmTexture::GetTexture(field.data.int_0);
                jfield["data"] = texcube->engineId;
                if (!cubetexs.Contains(texcube))
                {
                    cubetexs.Add(texcube);
                }
            }
            else if (field.type == tmMaterial::SAMPLER2D)
            {
                var tex = tmTexture::GetTexture(field.data.int_0);
                string name = field.name;
                jfield["data"] = field.data.int_0;

                std::cout << field.name << ":" << tex->engineId << std::endl;

                if (!texs.Contains(tex))
                {
                    texs.Add(tex);
                }
            }
            else if (field.type == tmMaterial::INT)
            {
                var data_int = field.data.int_0;
                glm::to_json(jfield["data"], data_int);
            }
            else if (field.type == tmMaterial::FLOAT)
            {
                var data_float = field.data.float_0;
                glm::to_json(jfield["data"], data_float);
            }
            else if (field.type == tmMaterial::BOOL)
            {
                var data_bool = field.data.bool_0;
                glm::to_json(jfield["data"], data_bool);
            }
            else if (field.type == tmMaterial::VEC2)
            {
                var data_vec2 = field.data.vec2_0;
                glm::to_json(jfield["data"], data_vec2);
            }
            else if (field.type == tmMaterial::VEC3)
            {
                var data_vec3 = field.data.vec3_0;
                glm::to_json(jfield["data"], data_vec3);
            }
            else if (field.type == tmMaterial::VEC4)
            {

                var data_vec4 = field.data.vec4_0;
                glm::to_json(jfield["data"], data_vec4);
            }
            else if (field.type == tmMaterial::MAT2)
            {
                continue;
                var data_mat2 = field.data.mat2_0;
                glm::to_json(jfield["data"], data_mat2);
            }
            else if (field.type == tmMaterial::MAT3)
            {
                continue;
                var data_mat3 = field.data.mat3_0;
                glm::to_json(jfield["data"], data_mat3);
            }
            else if (field.type == tmMaterial::MAT4) // ERROR HERE, FIELD IS UNDEFINED
            {
                continue;
                var data_mat4 = field.data.mat4_0;
                glm::to_json(jfield["data"], data_mat4);
            }

            m["fields"].push_back(jfield);

            //std::cout << "done" << std::endl;

        }

        j["materials"][i] = m;
    }

    for (auto tex : tmTexture::textures.GetVector())
    {
        nlohmann::json t;

        t["path"] = tex.second->path;
        t["flip"] = tex.second->flip;

        j["textures"].push_back(t);
    }

    for (auto tex : tmCubemap::cb_textures.GetVector())
    {
        nlohmann::json t;

        t["path"] = tex->path;
        t["flip"] = tex->flip;

        j["cube_textures"].push_back(t);
    }

    return j;
}



tmRenderMgr::tmRenderMgr()
{
    var blackTexture = new tmTexture(vec3(0, 0, 0), 10, 10);
}

tmRenderMgr::~tmRenderMgr()
{
    for (auto draw_call : drawCalls)
        delete draw_call;

    for (auto material : materials)
        delete material;

    for (auto texture : tmTexture::textures)
    	texture.second->unload();

    for (auto shader : tmShader::shader_index)
        delete shader;

    for (std::pair<std::string, tmModel*> loaded_model : tmModel::loadedModels)
    {
        delete loaded_model.second;
    }

    for (auto animation_index : tmAnimation::animationIndex)
        delete animation_index;
}

tmDrawCall* tmRenderMgr::InsertCall(tmVertexBuffer* buffer, unsigned material, std::vector<mat4> boneTransforms, glm::mat4 modelMatrix, tmActor* actor)
{
    var call = new tmDrawCall(buffer, material);
    call->modelMatrix = modelMatrix;
    call->actor = actor;
    call->boneMats = boneTransforms;

    drawCalls.push_back(call);

    return call;
}

void tmRenderMgr::Draw(tmBaseCamera* currentCamera)
{
    for (auto draw_call : drawCalls)
    {

        var material = materials[draw_call->material];

        int textureI = 0;

        var shader = material->shader;

        for (auto field : material->fields)
        {

            if (field.name == "model" || field.name == "projection" || field.name == "view") {
                continue;
            }

            if (draw_call->overrideFields.Contains(field.name))
                continue;

	        if (field.type == tmMaterial::INT)
                shader->setInt(field.name, field.data.int_0);
                
	        if (field.type == tmMaterial::FLOAT)
                shader->setFloat(field.name, field.data.float_0);
                
	        if (field.type == tmMaterial::BOOL)
                shader->setInt(field.name, field.data.bool_0);
                
	        if (field.type == tmMaterial::VEC2)
                shader->setVec2(field.name, field.data.vec2_0);
                
	        if (field.type == tmMaterial::VEC3)
                shader->setVec3(field.name, field.data.vec3_0);
                
	        if (field.type == tmMaterial::VEC4)
                shader->setVec4(field.name, field.data.vec4_0);
                
	        if (field.type == tmMaterial::MAT2)
                shader->setMat2(field.name, field.data.mat2_0);
                
	        if (field.type == tmMaterial::MAT3)
                shader->setMat3(field.name, field.data.mat3_0);
                
	        if (field.type == tmMaterial::MAT4)
                shader->setMat4(field.name, field.data.mat4_0);
                
            if (field.type == tmMaterial::SAMPLER2D) {
                shader->setInt(field.name, textureI);
                glActiveTexture(GL_TEXTURE0 + textureI);
                glBindTexture(GL_TEXTURE_2D, tmTexture::GetTexture(field.data.int_0)->id);
                textureI++;
            }
                
            if (field.type == tmMaterial::SAMPLERCUBE) {
                shader->setInt(field.name, textureI);
                glActiveTexture(GL_TEXTURE0 + textureI);
                glBindTexture(GL_TEXTURE_CUBE_MAP, tmTexture::GetTexture(field.data.int_0)->id);
                textureI++;
            }
        }

        for (auto [fieldName, field] : draw_call->overrideFields)
        {

            if (field.type == tmMaterial::INT)
                shader->setInt(field.name, field.data.int_0);

            if (field.type == tmMaterial::FLOAT)
                shader->setFloat(field.name, field.data.float_0);

            if (field.type == tmMaterial::BOOL)
                shader->setInt(field.name, field.data.bool_0);

            if (field.type == tmMaterial::VEC2)
                shader->setVec2(field.name, field.data.vec2_0);

            if (field.type == tmMaterial::VEC3)
                shader->setVec3(field.name, field.data.vec3_0);

            if (field.type == tmMaterial::VEC4)
                shader->setVec4(field.name, field.data.vec4_0);

            if (field.type == tmMaterial::MAT2)
                shader->setMat2(field.name, field.data.mat2_0);

            if (field.type == tmMaterial::MAT3)
                shader->setMat3(field.name, field.data.mat3_0);

            if (field.type == tmMaterial::MAT4)
                shader->setMat4(field.name, field.data.mat4_0);

            if (field.type == tmMaterial::SAMPLER2D) {
                shader->setInt(field.name, textureI);
                glActiveTexture(GL_TEXTURE0 + textureI);
                glBindTexture(GL_TEXTURE_2D, (field.data.int_0));
                textureI++;
            }

            if (field.type == tmMaterial::SAMPLERCUBE) {
                shader->setInt(field.name, textureI);
                glActiveTexture(GL_TEXTURE0 + textureI);
                glBindTexture(GL_TEXTURE_CUBE_MAP, tmTexture::GetTexture(field.data.int_0)->id);
                textureI++;
            }
        }

        material->shader->use();
        material->shader->setMat4("model",draw_call->modelMatrix);

        material->shader->setMat4Array("boneTransforms_NOREN", draw_call->boneMats);


        if (draw_call->actor) {
            material->shader->setFloat("actorIndex_NOREN", static_cast<float>(draw_call->actor->id));
            material->shader->setFloat("actorCount_NOREN", static_cast<float>(tmeGetCore()->GetActiveScene()->actorMgr->GetActorCount()));
        }

        tmeGetCore()->lighting->Update(material->shader);

        glBindVertexArray(draw_call->buffer->VAO);

        
        if (draw_call->subData.size() != 0)
        {
            //var subdata = static_cast<const float*>(draw_call->subData);
            glBindBuffer(GL_ARRAY_BUFFER, draw_call->buffer->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, draw_call->subDataSize, draw_call->subData.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        

        if (draw_call->buffer->EBO != -1)
        {
            glDrawElements(GL_TRIANGLES, draw_call->buffer->elementCount, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(draw_call->buffer->mode, 0, draw_call->buffer->vertexCount);
        }
        glBindVertexArray(0);
    }

    for (auto callback : callbacks)
    {
        callback(currentCamera);
    }
}

tmTexture::tmTexture(string path, bool flip, bool append, tmTextureSettings settings)
{
    std::replace(path.begin(), path.end(), '/', '\\');

    path = fs::path(path).lexically_normal().string();

    this->flip = flip;
    this->path = path;
    this->type = IMAGE;
    this->settings_ = settings;

    unsigned int texture;

    if (CheckTexture(path))
    {
        var tex = CheckTexture(path);


        id = tex->id;
        texture = tex->id;
        engineId = tex->engineId;

        if (path == tex->path && this->flip == tex->flip && this->settings_ == tex->settings_) {
            this->path = tex->path;
            this->flip = tex->flip;
            this->settings_ = tex->settings_;


            return;
        } else
        {
            textures[path] = this;
        }
    } else
    {
        glGenTextures(1, &texture);
        engineId = textures.GetCount();
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    // load and generate the texture

    stbi_set_flip_vertically_on_load(flip);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {

        GLenum format;

        switch (nrChannels)
        {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            break;
        }

        if (settings.type == 1)
        {
	        for (int i = 0; i < width * height * nrChannels; i += nrChannels)
	        {
                data[i+2] = (unsigned char)255;
	        }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        Logger::Log("Failed to load texture at " + path, Logger::LOG_ERROR, "TEXTURE");
    }
    stbi_image_free(data);

    id = texture;
    if (append) {
        textures[path] = this;
    }
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO, quadEBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float vertices[] = {
            // positions          // colors           // texture coords
             1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
             1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

tmTexture::tmTexture(tmShader* shader, int width, int height, int format)
{
    this->type = SHADER;
    this->path = shader->vertData + "|_|" + shader->fragData;
    engineId = textures.GetCount();


    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glViewport(0, 0, width, height);
    shader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    id = texture;
    flip = false;
    //textures.Add(this);
}

tmTexture::~tmTexture()
{
    var tex = textures[path];


    if (textures.Contains(this)) {

        int eid = engineId;

        glDeleteTextures(1, &id);

        textures.Remove(this);
        int i = 0;
        for (var tex : textures.GetVector())
        {
            if (tex.second->engineId > engineId)
                tex.second->engineId--;
            i++;
        }
    }

}

tmTexture::tmTexture(vec3 c, int width, int height)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Step 2: Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Step 3: Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Step 4: Create a solid color data array
    GLubyte* solidColorData = new GLubyte[width * height * 4];
    for (int i = 0; i < width * height * 4; i += 4) {
        solidColorData[i] = c.r*255;
        solidColorData[i + 1] = c.g*255;
        solidColorData[i + 2] = c.b*255;
        solidColorData[i + 3] = 255;
    }

    // Step 5: Upload the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, solidColorData);

    // Step 6: Generate mipmaps (optional)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Clean up the solid color data array
    delete[] solidColorData;

    id = textureID;

    this->type = IMAGE;
	engineId = textures.GetCount();
    textures.Add("",this);
}

void tmTexture::use(int offset)
{
    glActiveTexture(GL_TEXTURE0+offset);
    glBindTexture(GL_TEXTURE_2D, id);
}

void tmTexture::unload()
{
    glDeleteTextures(1, &id);
}

tmTexture* tmTexture::GetTexture(int i)
{
    return textures[i].second;
}

tmTexture* tmTexture::CheckTexture(string path)
{
    path = fs::path(path).lexically_normal().string();
    if (textures.Contains(path))
        return textures[path];

    return nullptr;

}

tmFramebuffer::tmFramebuffer(FramebufferType type, glm::vec2 size)
{
    this->framebuffer_type = type;
    this->type = FRAMEBUFFER;

    recreate(size);

}

void tmFramebuffer::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameId);

    var engine = tmeGetCore();

    glViewport(0, 0, size.x, size.y);

    if (framebuffer_type != DepthOnly) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);
    } else
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

void tmFramebuffer::draw()
{
    if (drawVAO == -1)
    {
        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        unsigned vao = tmgl::genVertexArray();
        unsigned vbo = tmgl::genBuffer(GL_ARRAY_BUFFER, quadVertices    , sizeof(quadVertices), GL_STATIC_DRAW);
        tmgl::genVertexBuffer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float));
        tmgl::genVertexBuffer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        //unsigned ebo = tmgl::genBuffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), GL_STATIC_DRAW);
        glBindVertexArray(0);

        drawVAO = vao;

        drawShader = new tmShader(("resources/shaders/quad/vertex.glsl"), ("resources/shaders/quad/fragment.glsl"),true);
    }

    glDisable(GL_DEPTH_TEST);

    drawShader->setInt("albedo", 0);
    drawShader->setInt("normal", 1);
    drawShader->setInt("position", 2);
    drawShader->setInt("shading", 3);
    drawShader->setInt("shadowPosition", 4);
    drawShader->setInt("skybox", 5);
    drawShader->setInt("irradiance", 6);
    drawShader->setInt("prefilter", 7);
    drawShader->setInt("brdfLUT", 8);
    drawShader->setInt("shadowMap", 9);

    drawShader->setVec3("clearColor", clearColor);
    drawShader->setVec3("cameraPosition", cameraPosition);

    tmeGetCore()->lighting->Update(drawShader);

    drawShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gAlb);
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, gNrm);
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, gPos);
    glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_2D, gSha);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, gLPos);

    if (tmeGetCore()->lighting->skybox)
    {
        var sky = tmeGetCore()->lighting->skybox;

        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_CUBE_MAP,sky->cubemap_->id);

        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky->cubemap_->irradianceId);

        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky->cubemap_->prefilterId);

        var tex = tmLighting::GetBRDFTexture();

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }

    glActiveTexture(GL_TEXTURE0 + 9);
    glBindTexture(GL_TEXTURE_2D, tmeGetCore()->lighting->depthFBO->gDepth);

    glBindVertexArray(drawVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void tmFramebuffer::reload()
{
    glDeleteVertexArrays(1,&drawVAO);
    drawShader->unload();
    drawVAO = -1;
}

void tmFramebuffer::recreate(glm::vec2 screenSize)
{
    if (frameId != -1)
    {
        unload();
    }

    size = screenSize;

    glGenFramebuffers(1, &frameId);
    glBindFramebuffer(GL_FRAMEBUFFER, frameId);

    if (framebuffer_type == Deferred) {

        // position color buffer
        glGenTextures(1, &gAlb);
        glBindTexture(GL_TEXTURE_2D, gAlb);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlb, 0);
        // normal color buffer
        glGenTextures(1, &gNrm);
        glBindTexture(GL_TEXTURE_2D, gNrm);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNrm, 0);
        // color + specular color buffer
        glGenTextures(1, &gPos);
        glBindTexture(GL_TEXTURE_2D, gPos);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gPos, 0);
        glGenTextures(1, &gSha);
        glBindTexture(GL_TEXTURE_2D, gSha);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSha, 0);
        glGenTextures(1, &gLPos);
        glBindTexture(GL_TEXTURE_2D, gLPos);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gLPos, 0);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        const unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
        glDrawBuffers(5, attachments);

        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenSize.x, screenSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        //delete attachments;
    }
    else if (framebuffer_type == Color)
    {
        // color + specular color buffer
        glGenTextures(1, &gAlb);
        glBindTexture(GL_TEXTURE_2D, gAlb);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlb, 0);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        const unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);
        //delete attachments;
            // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenSize.x, screenSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    }
    else if (framebuffer_type == DepthOnly)
    {
        glGenTextures(1, &gDepth);
        glBindTexture(GL_TEXTURE_2D, gDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            screenSize.x, screenSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Logger::Log("Framebuffer not complete!", Logger::Level::DBG);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tmFramebuffer::unload()
{
    if (framebuffer_type == Deferred) {

        glDeleteTextures(1, &gAlb);
        glDeleteTextures(1, &gNrm);
        glDeleteTextures(1, &gPos);
        glDeleteTextures(1, &gSha);
        glDeleteTextures(1, &gLPos);
    }
    else if (framebuffer_type == Color)
    {
        glDeleteTextures(1, &gAlb);
    } else if (framebuffer_type == DepthOnly)
    {
        glDeleteTextures(1, &gDepth);
    }


    glDeleteRenderbuffers(1, &rboDepth);
    glDeleteFramebuffers(1, &frameId);
}
