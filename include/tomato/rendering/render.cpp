#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "engine.hpp"
#include "stb/stb_image.h"
#include "util/filesystem_tm.h"

CLASS_DEFINITION(Component, tmCamera)
CLASS_DEFINITION(Component, tmRenderer)
CLASS_DEFINITION(tmRenderer, tmMeshRenderer)

Dictionary<string, tmModel*> tmModel::loadedModels;
tmCamera* tmCamera::_mainCamera;

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

tmShader::tmShader(string vertex, string fragment)
{
	id = tmgl::genShader(vertex.c_str(), fragment.c_str());
    vertData = vertex;
    fragData = fragment;
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

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    var cameraFront = glm::normalize(direction);

    //cameraFront = actor->transform->GetForward();
    //cameraFront = { 0,0,-1 };

    var pos = actor->transform->position;

    //pos = { 0,0,3 };

    view = glm::lookAt(pos, pos+cameraFront, actor->transform->GetUp());

    var engine = tmeGetCore();

    float aspect = engine->ScreenWidth / engine->ScreenHeight;

    switch (Projection)
    {
    case Perspective:
        proj = glm::perspective(glm::radians(FieldOfView), aspect, NearPlane, FarPlane);
        break;
    case Orthographic:
        //proj = glm::ortho
        break;
    }



}

void tmCamera::LateUpdate()
{
    // drawing
    Use();

    for (auto material : tmeGetCore()->renderMgr->materials)
        UpdateShader(material->shader);

    var renderMgr = tmeGetCore()->renderMgr;
    renderMgr->Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    c["framebuffer_type"] = framebuffer->type;

    return c;
}

tmCamera* tmCamera::GetMainCamera()
{
    return _mainCamera;
}

void tmBaseCamera::UpdateShader(tmShader* shader)
{
    shader->setMat4("view", view);
    shader->setMat4("projection", proj);
}


tmVertexBuffer::tmVertexBuffer(tmVertex* data,size_t count, size_t elementCount, unsigned int* indices)
{
    unsigned vao = tmgl::genVertexArray();
    unsigned vbo = tmgl::genBuffer(GL_ARRAY_BUFFER, &data[0],  count * sizeof(tmVertex), GL_STATIC_DRAW);
    if (indices != nullptr) EBO = tmgl::genBuffer(GL_ELEMENT_ARRAY_BUFFER, &indices[0], elementCount * sizeof(unsigned int), GL_STATIC_DRAW);
    tmgl::genVertexBuffer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex));
    tmgl::genVertexBuffer(1, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,normal));
    tmgl::genVertexBuffer(2, 2, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,texcoords));
    glBindVertexArray(0);

    VAO = vao;
    VBO = vbo;
    vertexCount = count;
    this->elementCount = elementCount;
    shader = 0;
}


tmMaterial::tmMaterial(tmShader* shader)
{
    materialIndex = tmeGetCore()->renderMgr->materials.size();
    tmeGetCore()->renderMgr->materials.push_back(this);
    this->shader = shader;
}

tmMesh::tmMesh(tmVertex* verts, size_t vertCount, size_t elemCount, unsigned* ind)
{
    vertices = verts;
    indices = ind;
    vertexCount = vertCount;
    elementCount = elemCount;

    setup();

}

tmDrawCall* tmMesh::draw(unsigned material, glm::mat4 modelMatrix)
{
    return tmeGetCore()->renderMgr->InsertCall(&buffer, material, modelMatrix);
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
    var drawCall = mesh->draw(materialIndex, GetActor()->transform->GetMatrix());
}

void tmMeshRenderer::Initialize(tmModel* model, int meshIndex)
{
    mesh = model->meshes[meshIndex];
    this->meshIndex = meshIndex;
    this->model = model;
}

tmModel* tmModel::LoadModel(string path)
{

    if (loadedModels.Contains(path)) {
        return loadedModels[path];
    }

    return new tmModel(path);
}

tmModel::tmModel(string path)
{
    loadModel(path);
}

void tmModel::loadModel(string path)
{

    this->path = path;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ForceGenNormals |  aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
	    Logger::logger << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    //directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void tmModel::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.Add(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

tmMesh* tmModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<tmVertex> vertices;
	std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        tmVertex vertex;
        // process vertex positions, normals and texture coordinates

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
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
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    return new tmMesh(vertices.data(), vertices.size(),indices.size(), indices.data());
}

void tmRenderMgr::DeserializeGame(nlohmann::json g)
{

    std::vector<tmTexture*> texs;

    for (auto t : g["textures"])
    {
        texs.push_back(new tmTexture(t["path"], t["flip"]));
    }

    for (auto j : g["materials"])
    {
        var shader = new tmShader(j["vertex"], j["fragment"]);

        var mat = new tmMaterial(shader);

        if (!j["textures"].is_null()) {
            for (auto t : j["textures"])
                mat->textures.push_back(texs[t.get<int>()]);
        }

    }
}

nlohmann::json tmRenderMgr::SerializeGame()
{

    nlohmann::json j;

    List<tmTexture*> texs = List<tmTexture*>();

    for (auto material : materials)
    {
        nlohmann::json m;

        m["vertex"] = material->shader->vertData;
        m["fragment"] = material->shader->fragData;

        m["textures"] = nlohmann::json();

        for (auto texture : material->textures) {
            m["textures"].push_back(texs.GetCount());

            if (!texs.Contains(texture))
            {
                texs.Add(texture);
            }

        }

        j["texture_count"] = material->textures.size();

        j["materials"].push_back(m);
    }

    for (auto tex : texs.GetVector())
    {
        nlohmann::json t;

        t["path"] = tex->path;
        t["flip"] = tex->flip;

        j["textures"].push_back(t);
    }

    return j;
}

tmDrawCall* tmRenderMgr::InsertCall(tmVertexBuffer* buffer, unsigned material, glm::mat4 modelMatrix)
{
    var call = new tmDrawCall(buffer, material);
    call->modelMatrix = modelMatrix;

    drawCalls.push_back(call);

    return call;
}

void tmRenderMgr::Draw()
{
    for (auto draw_call : drawCalls)
    {

        var material = materials[draw_call->material];
        int i = 0;
        for (auto texture : material->textures)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture->id);
            i++;
        }
        material->shader->use();
        material->shader->setMat4("model",draw_call->modelMatrix);

        glBindVertexArray(draw_call->buffer->VAO);
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
}

tmTexture::tmTexture(string path, bool flip)
{
    this->flip = flip;
    this->path = path;

    unsigned int texture;
    glGenTextures(1, &texture);
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
            format = GL_R;
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

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        //Logger::Log("Failed to load texture at " + path, Logger::LOG_ERROR, "TEXTURE");
    }
    stbi_image_free(data);

    id = texture;
}

void tmTexture::use(int offset)
{
    glActiveTexture(GL_TEXTURE0+offset);
    glBindTexture(GL_TEXTURE_2D, id);
}

tmFramebuffer::tmFramebuffer(FramebufferType type, glm::vec2 size)
{
    this->type = type;

    recreate(size);

}

void tmFramebuffer::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameId);

    var engine = tmeGetCore();

    glViewport(0, 0, size.x, size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

        drawShader = new tmShader(tmfs::loadFileString("resources/shaders/quad/vertex.glsl"), tmfs::loadFileString("resources/shaders/quad/fragment.glsl"));
        drawShader->setInt("position", 2);
        drawShader->setInt("normal", 1);
        drawShader->setInt("albedo", 0);
        drawShader->setInt("shading", 3);
    }

    glDisable(GL_DEPTH_TEST);

    drawShader->setVec3("clearColor", clearColor);
    drawShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gAlb);
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, gNrm);
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, gPos);
    glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_2D, gSha);
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

    if (type == Deferred) {

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSha, 0);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        const unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);
        //delete attachments;
    }
    else if (type == Color)
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
    }
    // create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenSize.x, screenSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Logger::Log("Framebuffer not complete!", Logger::Level::DBG);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tmFramebuffer::unload()
{
    if (type == Deferred) {

        GLuint texs[] = {gPos,gAlb,gNrm,gSha};

        glDeleteTextures(4, texs);
    }
    else if (type == Color)
    {
        glDeleteTextures(1, &gAlb);
    }

    glDeleteRenderbuffers(1, &rboDepth);
    glDeleteFramebuffers(1, &frameId);
}
