#if !defined(PHYS_H)
#define PHYS_H
#include "engine.hpp"
#include "phys.h"
#include "phys.h"

#include "ecs/actor.h"

#include "glm/gtx/euler_angles.hpp"

struct Collider;
struct SphereCollider;
struct BoxCollider;
struct tmPhysicsBody;


struct CollisionResult {
	bool isColliding = false;
	glm::vec3 normal = vec3(0.0);
	float penetrationDepth = -1;

	Collider* other = nullptr;
	Collider* main = nullptr;

};


struct Collider
{
	enum ColliderType
	{
		Box,
		Sphere
	} type;

	tmPhysicsBody* body = nullptr;

	Collider();
	~Collider();

	virtual CollisionResult CheckCollision(BoxCollider other);
	virtual CollisionResult CheckCollision(SphereCollider other);

	virtual glm::mat3 CalculateInertiaTensor(float mass) { return mat3(0.0); }

private:

	tmPhysicsMgr* physicsMgr;
};

struct SphereCollider : Collider
{
	SphereCollider() {
		type = Sphere;
	}

	float radius = 1.f;

	CollisionResult CheckCollision(BoxCollider other) override;
	CollisionResult CheckCollision(SphereCollider other) override;

	glm::mat3 CalculateInertiaTensor(float mass) override;
};


struct BoxCollider : Collider
{

	BoxCollider()
	{
		type = Box;
	}

	vec3 size = vec3(1);

	glm::vec3 getHalfSize() const {
		return size / 2.0f;
	}
	glm::mat3 getOrientationMatrix() const;

	CollisionResult CheckCollision(BoxCollider other) override;
	CollisionResult CheckCollision(SphereCollider other) override;

	std::vector<glm::vec3> getVertices() const;

	glm::mat3 CalculateInertiaTensor(float mass) override;
};



struct tmPhysicsBody
{
	vec3 position = vec3(0.0), rotation = vec3(0.0);

	vec3 velocity = vec3(0.0), angularVelocity = vec3(0.0), force = vec3(0.0);

	glm::mat3 inertiaTensor; // Inertia tensor for box colliders
	glm::mat3 invInertiaTensor; // Inverse of inertia tensor for box colliders

	float mass = 1.0f;
	float restitution = 0.5f;

	bool isStatic = false;
	float invMass;

	void applyForce(const glm::vec3& f) {
		force += f;
	}

	void applyTorque(const glm::vec3& torque);

	glm::mat3 getOrientationMatrix() const;

	void update(float deltaTime);

	tmPhysicsBody();
	~tmPhysicsBody();

	Collider* collider = nullptr;

private:

	tmPhysicsMgr* physicsMgr;

};

struct tmPhysicsMgr {

    tmPhysicsMgr();

	vec3 gravity = vec3(0, -9.81f/2, 0);

	void ResolveCollision(CollisionResult result);
    void Update();

	List<Collider*> colliders;
	List<tmPhysicsBody*> bodies;

private:

	void ResolveCollision(SphereCollider* c1, SphereCollider* c2, CollisionResult result);
	void ResolveCollision(SphereCollider* c1, BoxCollider* c2, CollisionResult result);
	void ResolveCollision(BoxCollider* c1, BoxCollider* c2, CollisionResult result);

};

class tmRigidBody : public Component
{
	CLASS_DECLARATION(tmRigidBody)

public:
	tmRigidBody(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmRigidBody() = default;
	void Start() override;
	void Update() override;
	void LateUpdate() override;
	void EngineRender() override;

	tmPhysicsBody* body;

};

QUICKCLASS(Component, tmCollider)
	
};

class tmBoxCollider : public tmCollider
{
	CLASS_DECLARATION(tmBoxCollider)

public:
	tmBoxCollider(std::string&& initialValue) : tmCollider(move(initialValue))
	{
	}

	tmBoxCollider() = default;
	void Update() override;
	void Start() override;
	void EngineRender() override;
	
	vec3 size;

private:
	BoxCollider* collider;

};

class tmSphereCollider : public tmCollider
{
	CLASS_DECLARATION(tmSphereCollider)

public:
	tmSphereCollider(std::string&& initialValue) : tmCollider(move(initialValue))
	{
	}

	tmSphereCollider() = default;
	void Update() override;
	void Start() override;
	void EngineRender() override;

	float radius;

private:
	SphereCollider* collider;

};

#endif // PHYS_H
