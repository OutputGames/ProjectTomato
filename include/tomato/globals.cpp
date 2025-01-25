#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"

#include "globals.hpp"

void glfw_errorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
};

tmt::render::RendererInfo* renderer;
std::vector<tmt::render::DrawCall> calls;
std::vector<tmt::debug::DebugCall> debugCalls;
std::vector<tmt::light::Light*> lights;
std::vector<std::function<void()>> debugFuncs;
glm::vec2 mousep;
glm::vec2 mousedelta;
glm::vec2 mousescrl;
int lastKey;
tmt::render::Camera* mainCamera;
tmt::obj::Scene* mainScene = nullptr;
tmt::render::Shader* defaultShader;
tmt::engine::Application* application;
int counterTime = 0;
float deltaTime = 1.0f / 60.0f;
float lastTime = 0;
u32 frameTime = 0;
bgfx::UniformHandle orthoHandle;
bgfx::UniformHandle timeHandle;
bgfx::UniformHandle vposHandle;
bgfx::UniformHandle animHandle;
tmt::light::LightUniforms* lightUniforms;
bool subHandlesLoaded = false;
std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*> primitives;

std::tuple<tmt::render::Vertex*, u16*, u16, u16> convertMesh(par_shapes_mesh* mesh)
{
    par_shapes_compute_normals(mesh);

    // Number of vertices and indices
    var vertexCount = mesh->npoints;
    var indexCount = mesh->ntriangles * 3;

    // Allocate memory for vertices and indices
    var vertices = new tmt::render::Vertex[vertexCount];
    var indices = new uint16_t[indexCount];

    // Populate vertices
    for (int i = 0; i < vertexCount; ++i)
    {
        tmt::render::Vertex& vertex = vertices[i];

        // Extract position
        vertex.position = glm::vec3(
            mesh->points[3 * i + 0],
            mesh->points[3 * i + 1],
            mesh->points[3 * i + 2]
            );

        // Extract normal if available
        if (mesh->normals)
        {
            vertex.normal = glm::vec3(
                mesh->normals[3 * i + 0],
                mesh->normals[3 * i + 1],
                mesh->normals[3 * i + 2]
                );
        }

        // Extract UV coordinates if available
        if (mesh->tcoords)
        {
            vertex.uv0 = glm::vec2(
                mesh->tcoords[2 * i + 0],
                mesh->tcoords[2 * i + 1]
                );
        }
    }

    // Populate indices
    for (int i = 0; i < mesh->ntriangles; ++i)
    {
        indices[3 * i + 0] = mesh->triangles[3 * i + 0];
        indices[3 * i + 1] = mesh->triangles[3 * i + 1];
        indices[3 * i + 2] = mesh->triangles[3 * i + 2];
    }
    return {vertices, indices, vertexCount, indexCount};
};

tmt::obj::CameraObject* mainCameraObject;

tmt::obj::Object* LoadObject(aiNode* node, tmt::obj::ObjectLoader::SceneInfo info)
{
    var obj = new tmt::obj::Object();
    obj->name = node->mName.C_Str();

    var transform = node->mTransformation;
    aiVector3D p, r, s;

    transform.Decompose(s, r, p);

    r.x = glm::degrees(r.x);
    r.y = glm::degrees(r.y);
    r.z = glm::degrees(r.z);

    obj->position = tmt::math::convertVec3(p);
    obj->rotation = tmt::math::convertVec3(r);
    obj->scale = tmt::math::convertVec3(s);

    for (int i = 0; i < node->mNumMeshes; ++i)
    {
        var meshIndex = node->mMeshes[i];

        var msh = new tmt::obj::MeshObject();
        msh->mesh = info.meshes[meshIndex];
        msh->material = info.materials[info.scene->mMeshes[meshIndex]->mMaterialIndex];

        msh->SetParent(obj);
    }

    for (int i = 0; i < node->mNumChildren; ++i)
    {
        var cobj = LoadObject(node->mChildren[i], info);

        cobj->SetParent(obj);
    }

    return obj;
};
std::vector<int> mstates(8);
std::vector<int> kstates;
std::vector<int> gstates;
btDiscreteDynamicsWorld* dynamicsWorld;
std::vector<btRigidBody*> physicalBodies;
std::vector<btCollisionShape*> collisionObjs;
std::vector<tmt::physics::PhysicsBody*> bodies;
std::vector<tmt::particle::Particle*> managed_particles;
bool doneFirstPhysicsUpdate;

glm::vec3 convertVec3(btVector3 v)
{
    return glm::vec3{v.x(), v.y(), v.z()};
};

btVector3 convertVec3(glm::vec3 v)
{
    return btVector3(v.x, v.y, v.z);
};

glm::vec3 convertQuatEuler(btQuaternion q)
{
    float x, y, z;

    q.getEulerZYX(x, y, z);

    return (glm::vec3{x, y, z});
};

btQuaternion convertQuat(glm::vec3 q)
{
    float x, y, z;

    var qu = radians(q);

    x = qu.x;
    y = qu.y;
    z = qu.z;

    return btQuaternion(y, x, z);
};

glm::quat convertQuat(btQuaternion q) { return glm::quat(q.w(), q.x(), q.y(), q.z()); }

btQuaternion convertQuat(glm::quat q)
{
    return btQuaternion(q.x, q.y, q.z, q.w);
};

void ApplyTransform(tmt::physics::PhysicsBody* body, btTransform transform)
{
    if (!body->active)
        return;

    var parent = body->parent;
    if (body->transRelation == tmt::physics::PhysicsBody::Self)
    {
        var p = body->position;

        body->position = convertVec3(transform.getOrigin());

        //transform.setOrigin(convertVec3(p));
    }
    else
    {
        var p = parent->position;
        var r = parent->rotation;

        parent->position = convertVec3(transform.getOrigin());

        parent->rotation = convertQuatEuler(transform.getRotation());
    }
};

bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                     glm::vec3 scale)
{
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 v2 = p - a;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    var bl = (u >= 0) && (v >= 0) && (u + v <= 1);

    if (bl)
    {
        //tmt::debug::Gizmos::DrawSphere(a, 1.f);
        //tmt::debug::Gizmos::DrawSphere(b, 1.f);
        //tmt::debug::Gizmos::DrawSphere(c, 1.f);
        //tmt::debug::Gizmos::DrawSphere(p, 1.f);
    }

    return bl;
};

glm::vec3 barycentricCoordinates(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                                 glm::vec3 scale)
{
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 v2 = p - a;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w = 1.0f - u - v;

    var bary = glm::vec3{u, v, w};

    //bary -= glm::vec3{ 0.5 };
    //bary = -bary;

    // Debug visualization (optional)
    if (u >= 0 && v >= 0 && w >= 0)
    {
        tmt::debug::Gizmos::DrawSphere(a, 1.f);
        tmt::debug::Gizmos::DrawSphere(b, 1.f);
        tmt::debug::Gizmos::DrawSphere(c, 1.f);
        tmt::debug::Gizmos::DrawSphere(p, 1.f);
        tmt::debug::Gizmos::DrawSphere(bary, 1.f);
    }

    return bary;
};

btCollisionShape* ShapeFromInfo(tmt::physics::ColliderInitInfo i)
{
    btCollisionShape* shape;
    switch (i.s)
    {
        case tmt::physics::Box:
            shape = new btBoxShape(convertVec3(i.bounds / 2.f));
            break;
        case tmt::physics::Sphere:
            shape = new btSphereShape(i.radius);
            break;
        case tmt::physics::Capsule:
            shape = new btCapsuleShape(i.radius, i.height);
            break;
        case tmt::physics::Mesh:
        {
            auto indices = new int[i.mesh->indexCount];

            for (int j = 0; j < i.mesh->indexCount; ++j)
            {
                indices[j] = i.mesh->indices[j];
            }

            auto vertices = new glm::vec3[i.mesh->vertexCount];

            for (int j = 0; j < i.mesh->vertexCount; ++j)
            {
                var vert = i.mesh->vertices[j].position;
                vertices[j] = vert;
            }

            auto indexVertexArray = new btTriangleIndexVertexArray(
                i.mesh->indexCount / 3, // Number of triangles
                &indices[0], // Pointer to index array
                sizeof(int) * 3, // Stride between indices
                i.mesh->vertexCount, // Number of vertices
                reinterpret_cast<float*>(&vertices[0]), // Pointer to vertex array
                sizeof(glm::vec3) // Stride between vertices (sizeof(float) * 3)
                );

            shape = new btBvhTriangleMeshShape(indexVertexArray, true);

            //shape = new btBvhTriangleMeshShape(triangleMesh, true);
        }
        break;
        default:
            break;
    }

    return shape;
};
