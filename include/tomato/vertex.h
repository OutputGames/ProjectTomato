#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

namespace tmt::render
{
    struct Vertex
    {
        glm::vec3 position = glm::vec3(0.0);
        glm::vec3 normal = glm::vec3(0);
        glm::vec2 uv0 = glm::vec2(0.5);
        glm::vec4 boneIds = glm::vec4(-1);
        glm::vec4 boneWeights = glm::vec4(0);

        static tmgl::VertexLayout getVertexLayout();

        void SetBoneData(int boneId, float boneWeight);
    };
}

#endif
