#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

CLASS_DEFINITION(Component, Camera)

tmShader::tmShader(string vertex, string fragment)
{
	id = tmgl::genShader(vertex.c_str(), fragment.c_str());
}

void Camera::Start()
{
	std::cout << "Camera initialized";
}

void Camera::Update()
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

    float aspect = 800.0f / 600.0f;

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

void Camera::UpdateShader(tmShader* shader)
{
    shader->setMat4("view", view);
    shader->setMat4("projection", proj);
}

void tmVertexBuffer::Draw()
{
    
    glUseProgram(shader);
    glBindVertexArray(VAO);
    if (EBO != -1)
    {
        glDrawElements(mode, vertexCount, elementType, 0);
    } else
    {
        glDrawArrays(mode, 0, vertexCount);
    }
    glBindVertexArray(0);
}

tmVertexBuffer::tmVertexBuffer(tmVertex* data,size_t count, unsigned int* indices)
{
    unsigned vao = tmgl::genVertexArray();
    unsigned vbo = tmgl::genBuffer(GL_ARRAY_BUFFER, &data[0],  count * sizeof(data), GL_STATIC_DRAW);
    tmgl::genVertexBuffer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex));
    tmgl::genVertexBuffer(1, 3, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,normal));
    tmgl::genVertexBuffer(2, 2, GL_FLOAT, GL_FALSE, sizeof(tmVertex), (void*)offsetof(tmVertex,texcoords));
    if (indices != nullptr) EBO = tmgl::genBuffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), GL_STATIC_DRAW);
    glBindVertexArray(0);

    VAO = vao;
    VBO = vbo;
    vertexCount = count;
    shader = 0;
}

tmTexture::tmTexture(string path, bool flip)
{
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
