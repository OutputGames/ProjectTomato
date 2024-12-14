#include "utils.hpp" 
#include "Render/Color.hpp" 
#include "Render/Color.hpp" 
#include "Render/Color.hpp" 
#include "Render/Color.hpp" 
#include "Render/RendererInfo.hpp" 
#include "Render/Camera.hpp" 
#include "Obj/Scene.hpp" 
#include "Render/Shader.hpp" 
#include "Obj/CameraObject.hpp" 
#include "Physics/ColliderInitInfo.hpp" 

tmt::render::Color tmt::render::Color::White = {1, 1, 1, 1};
tmt::render::Color tmt::render::Color::Blue = {0, 0, 1, 1};
tmt::render::Color tmt::render::Color::Green = {0, 1, 0, 1};
tmt::render::Color tmt::render::Color::Red = {1, 0, 0, 1};
static tmt::render::RendererInfo* renderer;
std::vector<tmt::render::DrawCall> calls;
std::vector<tmt::debug::Gizmos::DebugCall> debugCalls;
glm::vec2 mousep;
glm::vec2 mousedelta;
tmt::render::Camera* mainCamera;
tmt::obj::Scene* mainScene;
tmt::render::Shader* defaultShader;
int counterTime = 0;
float deltaTime = 1.0f / 60.0f;
float lastTime = 0;
bgfx::UniformHandle orthoHandle;
bool subHandlesLoaded = false;
std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*> primitives;
tmt::obj::CameraObject* mainCameraObject;
std::vector<int> mstates;
std::vector<int> kstates;
btDiscreteDynamicsWorld* dynamicsWorld;
std::vector<btRigidBody*> physicalBodies;
std::vector<btCollisionShape*> collisionObjs;
std::vector<tmt::physics::PhysicsBody*> bodies;
std::vector<tmt::particle::Particle*> managed_particles;
bool doneFirstPhysicsUpdate;
tmt::physics::ColliderInitInfo tmt::physics::ColliderInitInfo::ForBox;
