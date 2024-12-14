#include "Model.hpp" 
#include "globals.cpp" 

tmt::render::Model::Model(string path)
{
    Assimp::Importer import;
    const aiScene *scene =
        import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

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

        meshes.push_back(
            createMesh(vertices.data(), indices.data(), vertices.size(), indices.size(), Vertex::getVertexLayout()));
        materialIndices.push_back(msh->mMaterialIndex);
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

