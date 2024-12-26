#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "tomato.hpp"
#include "utils.hpp"
#include "bimg/include/bimg/bimg.h"
#include "bx/math.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#endif
#include <complex.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#ifndef PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"
#endif
#include "tomato_generated/cube.h"

void glfw_errorCallback(int error, const char* description);
extern tmt::render::RendererInfo* renderer;
extern std::vector<tmt::render::DrawCall> calls;
extern std::vector<tmt::debug::DebugCall> debugCalls;
extern std::vector<tmt::light::Light*> lights;
extern std::vector<std::function<void()>> debugFuncs;
extern glm::vec2 mousep;
extern glm::vec2 mousedelta;
extern tmt::render::Camera* mainCamera;
extern tmt::obj::Scene* mainScene;
extern tmt::render::Shader* defaultShader;
extern int counterTime;
extern float deltaTime;
extern float lastTime;
extern u32 frameTime;
extern bgfx::UniformHandle orthoHandle;
extern bgfx::UniformHandle timeHandle;
extern bgfx::UniformHandle vposHandle;
extern bgfx::UniformHandle animHandle;
extern tmt::light::LightUniforms* lightUniforms;
extern bool subHandlesLoaded;
extern std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*> primitives;
std::tuple<tmt::render::Vertex*, u16*, u16, u16> convertMesh(par_shapes_mesh* mesh);
extern tmt::obj::CameraObject* mainCameraObject;
tmt::obj::Object* LoadObject(aiNode* node, tmt::obj::ObjectLoader::SceneInfo info);
extern std::vector<int> mstates;
extern std::vector<int> kstates;
extern btDiscreteDynamicsWorld* dynamicsWorld;
extern std::vector<btRigidBody*> physicalBodies;
extern std::vector<btCollisionShape*> collisionObjs;
extern std::vector<tmt::physics::PhysicsBody*> bodies;
extern std::vector<tmt::particle::Particle*> managed_particles;
extern bool doneFirstPhysicsUpdate;
glm::vec3 convertVec3(btVector3 v);
btVector3 convertVec3(glm::vec3 v);
glm::vec3 convertQuatEuler(btQuaternion q);
btQuaternion convertQuat(glm::vec3 q);
btQuaternion convertQuat(glm::quat q);
glm::quat convertQuat(btQuaternion q);
void ApplyTransform(tmt::physics::PhysicsBody* body, btTransform transform);
bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                     glm::vec3 scale = glm::vec3{1});
glm::vec3 barycentricCoordinates(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                                 glm::vec3 scale = glm::vec3{1});
btCollisionShape* ShapeFromInfo(tmt::physics::ColliderInitInfo i);


#endif
