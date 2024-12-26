#include "prim.hpp"
#include "globals.hpp"

tmt::render::Mesh* tmt::prim::GetPrimitive(PrimitiveType type)
{
    if (!primitives.contains(type))
    {
        render::Vertex* vertices;
        u16* indices;

        size_t vertCount, indCount;

        switch (type)
        {
            case Quad:
            {
                vertices = new render::Vertex[4]{
                    {glm::vec3{0, 0, 0}, glm::vec3{1}, glm::vec2{0, 0}},
                    {glm::vec3{1, 0, 0}, glm::vec3{1}, glm::vec2{1, 0}},
                    {glm::vec3{1, 1, 0}, glm::vec3{1}, glm::vec2{1, 1}},
                    {glm::vec3{0, 1, 0}, glm::vec3{1}, glm::vec2{0, 1}},
                };

                indices = new u16[6]{
                    2, 3, 0, 0, 1, 2,
                };

                vertCount = 4;
                indCount = 6;
            }

            break;
            case Cube:
            {
                vertCount = cube_mesh::vertexCount_0;
                indCount = cube_mesh::indexCount_0;
                vertices = cube_mesh::vertices_0;
                indices = cube_mesh::indices_0;
            }
            break;
            case Sphere:
            {
                var mesh = par_shapes_create_parametric_sphere(12, 12);
                var [verts, inds, vc, ic] = convertMesh(mesh);
                vertCount = vc;
                indCount = ic;
                vertices = verts;
                indices = inds;
                par_shapes_free_mesh(mesh);
            }
            break;
        }

        var mesh = createMesh(vertices, indices, vertCount, indCount, render::Vertex::getVertexLayout());

        primitives.insert(std::make_pair(type, mesh));
    }

    return primitives[type];
}
