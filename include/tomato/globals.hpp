/**
 * @file globals.hpp
 * @brief Global variables, utility functions, and cross-system type conversions
 * 
 * This header declares global state variables used throughout the engine
 * and provides utility functions for:
 * - Converting between physics engine (Bullet3) and graphics (GLM) types
 * - Loading 3D models from Assimp format
 * - Geometric calculations (point-in-triangle, barycentric coordinates)
 * - Creating physics collision shapes
 */

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "tomato.hpp"
#include "utils.hpp"
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

// === GLFW Callbacks ===
/**
 * @brief Error callback for GLFW library errors
 * @param error GLFW error code
 * @param description Human-readable error description
 */
void glfw_errorCallback(int error, const char* description);

// === Core Engine State ===
extern tmt::render::RendererInfo* renderer;              ///< Main renderer instance
extern tmt::engine::Application* application;            ///< Current application instance
extern tmt::obj::Scene* mainScene;                       ///< Main scene containing game objects
extern tmt::obj::CameraObject* mainCameraObject;         ///< Main camera object for rendering

// === Rendering State ===
extern std::vector<tmt::debug::DebugCall> debugCalls;    ///< Debug draw calls (lines, spheres, etc.)
extern std::vector<tmt::render::DrawCall> drawCalls;     ///< Queued rendering draw calls
extern std::vector<tmt::light::Light*> lights;           ///< Active lights in the scene
extern tmt::render::Shader* defaultShader;               ///< Default shader used for rendering
extern tmt::light::LightUniforms* lightUniforms;         ///< Uniform buffer for lighting data
extern bool subHandlesLoaded;                             ///< Whether sub-shader handles are loaded

// === Shader Uniforms ===
extern tmgl::UniformHandle orthoHandle;                  ///< Orthographic projection uniform
extern tmgl::UniformHandle timeHandle;                   ///< Time uniform for animations
extern tmgl::UniformHandle vposHandle;                   ///< Vertex position uniform
extern tmgl::UniformHandle animHandle;                   ///< Animation uniform

// === Input State ===
extern glm::vec2 mousep;                                 ///< Current mouse position
extern glm::vec2 mousedelta;                             ///< Mouse movement delta this frame
extern glm::vec2 mousescrl;                              ///< Mouse scroll delta
extern int lastKey;                                      ///< Last key pressed
extern std::vector<int> mstates;                         ///< Mouse button states
extern std::vector<int> gstates;                         ///< Gamepad button states
extern std::vector<int> kstates;                         ///< Keyboard key states

// === Time State ===
extern int counterTime;                                  ///< Frame counter for time-based effects
extern float deltaTime;                                  ///< Time elapsed since last frame (seconds)
extern float lastTime;                                   ///< Time of last frame
extern u32 frameTime;                                    ///< Current frame time

// === Physics State ===
extern btDiscreteDynamicsWorld* dynamicsWorld;           ///< Bullet physics world
extern std::vector<btRigidBody*> physicalBodies;         ///< All rigid bodies in physics sim
extern std::vector<btCollisionShape*> collisionObjs;     ///< All collision shapes
extern std::vector<tmt::physics::PhysicsBody*> bodies;   ///< Engine physics body wrappers
extern bool doneFirstPhysicsUpdate;                      ///< Whether first physics update has occurred

// === Particle System ===
extern std::vector<tmt::particle::Particle*> managed_particles;  ///< Active particles

// === Debug Functions ===
extern std::vector<std::function<void()>> debugFuncs;    ///< Debug functions to execute

// === Primitive Meshes ===
extern std::map<tmt::prim::PrimitiveType, tmt::render::Mesh*> primitives;  ///< Cached primitive meshes

// === Mesh Conversion Functions ===

/**
 * @brief Convert a par_shapes mesh to engine vertex/index format
 * 
 * Takes a procedurally generated mesh from the par_shapes library and converts
 * it to the engine's vertex format with positions, normals, and UVs.
 * 
 * @param mesh The par_shapes mesh to convert
 * @return Tuple containing: (vertices array, indices array, vertex count, index count)
 */
std::tuple<tmt::render::Vertex*, u16*, u16, u16> convertMesh(par_shapes_mesh* mesh);

/**
 * @brief Load an object hierarchy from an Assimp scene node
 * 
 * Recursively loads a 3D object and its children from an Assimp scene.
 * Handles meshes, materials, and transforms.
 * 
 * @param node The Assimp scene node to load
 * @param info Scene information including meshes and materials
 * @return tmt::obj::Object* Loaded object with all children attached
 */
tmt::obj::Object* LoadObject(aiNode* node, tmt::obj::ObjectLoader::SceneInfo info);

// === Physics Type Conversions (GLM <-> Bullet3) ===

/**
 * @brief Convert Bullet3 vector to GLM vec3
 * @param v Bullet vector
 * @return glm::vec3 Equivalent GLM vector
 */
glm::vec3 convertVec3(btVector3 v);

/**
 * @brief Convert GLM vec3 to Bullet3 vector
 * @param v GLM vector
 * @return btVector3 Equivalent Bullet vector
 */
btVector3 convertVec3(glm::vec3 v);

/**
 * @brief Convert Bullet3 quaternion to Euler angles (GLM vec3)
 * @param q Bullet quaternion
 * @return glm::vec3 Euler angles in radians
 */
glm::vec3 convertQuatEuler(btQuaternion q);

/**
 * @brief Convert Euler angles (GLM vec3) to Bullet3 quaternion
 * @param q Euler angles in degrees
 * @return btQuaternion Equivalent Bullet quaternion
 */
btQuaternion convertQuat(glm::vec3 q);

/**
 * @brief Convert GLM quaternion to Bullet3 quaternion
 * @param q GLM quaternion
 * @return btQuaternion Equivalent Bullet quaternion
 */
btQuaternion convertQuat(glm::quat q);

/**
 * @brief Convert Bullet3 quaternion to GLM quaternion
 * @param q Bullet quaternion
 * @return glm::quat Equivalent GLM quaternion
 */
glm::quat convertQuat(btQuaternion q);

// === Physics Utilities ===

/**
 * @brief Apply a physics transform to a physics body
 * 
 * Updates the position and rotation of a physics body based on the
 * transform from the physics simulation.
 * 
 * @param body The physics body to update
 * @param transform The transform from physics simulation
 */
void ApplyTransform(tmt::physics::PhysicsBody* body, btTransform transform);

// === Geometric Utilities ===

/**
 * @brief Check if a point lies within a triangle
 * 
 * Uses barycentric coordinates to determine if a 3D point is inside
 * the triangle defined by vertices a, b, c.
 * 
 * @param p The point to test
 * @param a First triangle vertex
 * @param b Second triangle vertex
 * @param c Third triangle vertex
 * @param scale Optional scale factor for vertices (default: 1,1,1)
 * @return true if point is inside triangle, false otherwise
 */
bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                     glm::vec3 scale = glm::vec3{1});

/**
 * @brief Calculate barycentric coordinates of a point relative to a triangle
 * 
 * Barycentric coordinates represent the point as a weighted combination
 * of the triangle's vertices. Useful for interpolating vertex attributes.
 * 
 * @param p The point to calculate coordinates for
 * @param a First triangle vertex
 * @param b Second triangle vertex
 * @param c Third triangle vertex
 * @param scale Optional scale factor for vertices (default: 1,1,1)
 * @return glm::vec3 Barycentric coordinates (u, v, w) where u+v+w = 1
 */
glm::vec3 barycentricCoordinates(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                                 glm::vec3 scale = glm::vec3{1});

// === Physics Shape Creation ===

/**
 * @brief Create a Bullet3 collision shape from collider initialization info
 * 
 * Factory function that creates the appropriate Bullet collision shape
 * (box, sphere, capsule, or mesh) based on the provided parameters.
 * 
 * @param i Collider initialization information including type and dimensions
 * @return btCollisionShape* The created collision shape (caller owns memory)
 */
btCollisionShape* ShapeFromInfo(tmt::physics::ColliderInitInfo i);


#endif
