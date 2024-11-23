#include "phys.h"

#include <debug_draw.hpp>

#include "debug.h"
#include "engine.hpp"

CLASS_DEFINITION(Component, tmRigidBody)
CLASS_DEFINITION(Component, tmCollider)
CLASS_DEFINITION(tmCollider, tmBoxCollider)


CLASS_DEFINITION(tmCollider, tmSphereCollider)

Collider::Collider()
{
	physicsMgr = tmeGetCore()->GetActiveScene()->physicsMgr;

	physicsMgr->colliders.Add(this);
}

Collider::~Collider()
{
	physicsMgr->colliders.Remove(this);
}

CollisionResult Collider::CheckCollision(BoxCollider other)
{
	return {false};
}

CollisionResult Collider::CheckCollision(SphereCollider other)
{
	return {false};
}

CollisionResult SphereCollider::CheckCollision(BoxCollider box)
{
	CollisionResult result;
	result.isColliding = false;

	// Transform sphere center to box's local space
	glm::vec3 localCenter = glm::transpose(box.getOrientationMatrix()) * (body->position - box.body->position);

	// Get the half extents of the box
	glm::vec3 halfExtents = box.size / 2.f;

	// Clamp local center to the box's extents
	glm::vec3 closestPoint = glm::clamp(localCenter, -halfExtents, halfExtents);

	// Transform the closest point back to world space
	closestPoint = box.body->position + box.getOrientationMatrix() * closestPoint;

	// Calculate the distance between the sphere center and the closest point
	float distance = glm::distance(body->position, closestPoint);

	// Check for collision
	if (distance <= radius) {
		result.isColliding = true;
		result.penetrationDepth = radius - distance;
		result.normal = glm::normalize(body->position - closestPoint);
	}

	return result;
}

CollisionResult SphereCollider::CheckCollision(SphereCollider other)
{
	float distance = glm::distance(body->position, other.body->position);

	glm::vec3 normal = glm::normalize(other.body->position - this->body->position);
	float penetrationDepth = (this->radius + other.radius) - glm::distance(this->body->position, other.body->position);

	return {distance < (radius + other.radius), normal, penetrationDepth};
}

glm::mat3 SphereCollider::CalculateInertiaTensor(float mass)
{
	float i = 2.0f / 5.0f * mass * radius * radius;
	return glm::mat3(i);
}

float projectBoxOnAxis(const BoxCollider& box, const glm::vec3& axis) {
	std::vector<glm::vec3> vertices = box.getVertices();
	float min = glm::dot(vertices[0], axis);
	float max = min;
	for (const auto& vertex : vertices) {
		float projection = glm::dot(vertex, axis);
		if (projection < min) min = projection;
		if (projection > max) max = projection;
	}
	return max - min;
}

glm::mat3 BoxCollider::getOrientationMatrix() const
{
	return body->getOrientationMatrix();
}

CollisionResult TestAxis(const glm::vec3& axis, const BoxCollider& box1, const BoxCollider& box2) {
	glm::mat3 orientation1 = box1.getOrientationMatrix();
	glm::mat3 orientation2 = box2.getOrientationMatrix();

	// Project the full-size extents of both OBBs onto the axis
	float projection1 = glm::abs(glm::dot(orientation1[0] * box1.size.x * 0.5f, axis)) +
		glm::abs(glm::dot(orientation1[1] * box1.size.y * 0.5f, axis)) +
		glm::abs(glm::dot(orientation1[2] * box1.size.z * 0.5f, axis));
	float projection2 = glm::abs(glm::dot(orientation2[0] * box2.size.x * 0.5f, axis)) +
		glm::abs(glm::dot(orientation2[1] * box2.size.y * 0.5f, axis)) +
		glm::abs(glm::dot(orientation2[2] * box2.size.z * 0.5f, axis));

	// Calculate the distance between the box centers projected onto the axis
	float distance = glm::abs(glm::dot(box2.body->position - box1.body->position, axis));

	CollisionResult result;
	float overlap = projection1 + projection2 - distance;
	result.isColliding = (overlap > 0);
	result.normal = axis;
	result.penetrationDepth = overlap;

	return result;
}

CollisionResult BoxCollider::CheckCollision(BoxCollider other)
{
	glm::mat3 orientation1 = getOrientationMatrix();
	glm::mat3 orientation2 = other.getOrientationMatrix();

	glm::vec3 axes[15] = {
		orientation1[0], orientation1[1], orientation1[2],
		orientation2[0], orientation2[1], orientation2[2],
		glm::cross(orientation1[0], orientation2[0]),
		glm::cross(orientation1[0], orientation2[1]),
		glm::cross(orientation1[0], orientation2[2]),
		glm::cross(orientation1[1], orientation2[0]),
		glm::cross(orientation1[1], orientation2[1]),
		glm::cross(orientation1[1], orientation2[2]),
		glm::cross(orientation1[2], orientation2[0]),
		glm::cross(orientation1[2], orientation2[1]),
		glm::cross(orientation1[2], orientation2[2])
	};

	CollisionResult finalResult;
	finalResult.isColliding = true;
	finalResult.penetrationDepth = std::numeric_limits<float>::max();

	for (const auto& axis : axes) {
		if (glm::length(axis) > std::numeric_limits<float>::epsilon()) {

			//dd::line(glmtoddin(axis), glmtoddin((axis * 2.f)), dd::colors::Orange);

			CollisionResult result = TestAxis(glm::normalize(axis), *this, other);
			if (!result.isColliding) {
				finalResult.isColliding = false;
				return finalResult;
			}
			else if (result.penetrationDepth < finalResult.penetrationDepth) {
				finalResult = result;
			}
		}
	}

	finalResult.normal = glm::normalize(finalResult.normal);
	return finalResult;
}

CollisionResult BoxCollider::CheckCollision(SphereCollider other)
{
	return other.CheckCollision(*this);
}

std::vector<glm::vec3> BoxCollider::getVertices() const
{
	glm::vec3 halfSize = size / 2.0f;
	std::vector<glm::vec3> vertices = {
		glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
		glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
		glm::vec3(halfSize.x,  halfSize.y, -halfSize.z),
		glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
		glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
		glm::vec3(halfSize.x, -halfSize.y,  halfSize.z),
		glm::vec3(halfSize.x,  halfSize.y,  halfSize.z),
		glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z)
	};
	for (auto& v : vertices) {
		v = body->rotation * v + body->position;
	}
	return vertices;
}

glm::mat3 BoxCollider::CalculateInertiaTensor(float mass)
{
	float w = size.x;
	float h = size.y;
	float d = size.z;
	float m = mass / 12.0f;
	return glm::mat3(
		m * (h * h + d * d), 0, 0,
		0, m * (w * w + d * d), 0,
		0, 0, m * (w * w + h * h)
	);
}

void tmPhysicsBody::applyTorque(const glm::vec3& torque)
{
	glm::vec3 angularAcceleration = glm::inverse(inertiaTensor) * (torque);
	angularVelocity += angularAcceleration;
}

glm::mat3 tmPhysicsBody::getOrientationMatrix() const
{
	return glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
}

void tmPhysicsBody::update(float deltaTime)
{

	if (collider) {
		collider->body = this;
		inertiaTensor = collider->CalculateInertiaTensor(mass);
		invInertiaTensor = glm::inverse(inertiaTensor);
	}

	invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;

	if (!isStatic) {

		const float maxAngularVelocity = 10.0f; // Adjust this value as necessary
		if (glm::length(angularVelocity) > maxAngularVelocity) {
			angularVelocity = glm::normalize(angularVelocity) * maxAngularVelocity;
		}

		velocity += (force / mass) * deltaTime;

		if (glm::isnan(velocity)) velocity = vec3(0.0);

		position += velocity * deltaTime;

		glm::mat3 orientationMatrix = getOrientationMatrix();
		glm::vec3 angularAcceleration = ((invInertiaTensor) * (orientationMatrix * force));
		//angularVelocity += angularAcceleration;

		if (glm::isnan(angularVelocity)) angularVelocity = vec3(0.0);

		rotation += ((angularVelocity) * deltaTime);
	}
	force = glm::vec3(0.0f); // Reset force after each update
	applyForce(physicsMgr->gravity * mass);
}

tmPhysicsBody::tmPhysicsBody()
{
	physicsMgr = tmeGetCore()->GetActiveScene()->physicsMgr;

	physicsMgr->bodies.Add(this);
}

tmPhysicsBody::~tmPhysicsBody()
{
	physicsMgr->bodies.Remove(this);
}

tmPhysicsMgr::tmPhysicsMgr()
{
}

void tmPhysicsMgr::ResolveCollision(CollisionResult result)
{
	BoxCollider* box1 = nullptr;
	SphereCollider* sphere1 = nullptr;
	BoxCollider* box2 = nullptr;
	SphereCollider* sphere2 = nullptr;

	switch (result.main->type)
	{
	case Collider::Box:
		box1 = dynamic_cast<BoxCollider*>(result.main);
		break;
	case Collider::Sphere:
		sphere1 = dynamic_cast<SphereCollider*>(result.main);
		break;
	}

	switch (result.other->type)
	{
	case Collider::Box:
		box2 = dynamic_cast<BoxCollider*>(result.other);
		break;
	case Collider::Sphere:
		sphere2 = dynamic_cast<SphereCollider*>(result.other);
		break;
	}

	if (box1 && box2)
		ResolveCollision(box1, box2, result);
	if (sphere1 && sphere2)
		ResolveCollision(sphere1, sphere2, result);
	if (box1 && sphere1)
		ResolveCollision(sphere1, box1, result);
	if (box2 && sphere2)
		ResolveCollision(sphere2, box2, result);
	if (box1 && sphere2)
		ResolveCollision(sphere2, box1, result);
	if (box2 && sphere1)
		ResolveCollision(sphere1, box2, result);
}

void tmPhysicsMgr::Update()
{
	for (auto body : bodies)
	{
		body->update(tmEngine::tmTime::deltaTime);
	}

	for (auto collider : colliders)
	{

		if (!collider->body)
			continue;

		CollisionResult result = {false};

		for (auto otherCollider : colliders)
		{

			if (!otherCollider->body)
				continue;
			if (otherCollider == collider)
				continue;

			switch (otherCollider->type)
			{

			case Collider::Box:
				result = collider->CheckCollision(*dynamic_cast<BoxCollider*>(otherCollider));
				break;
			case Collider::Sphere:
				result = collider->CheckCollision(*dynamic_cast<SphereCollider*>(otherCollider));

				break;

			}

			if (result.isColliding)
			{
				dd::line(glmtoddin(collider->body->position), glmtoddin(otherCollider->body->position), dd::colors::Red);
				result.other = otherCollider;

				break;
			} else
			{
				dd::line(glmtoddin(collider->body->position), glmtoddin(otherCollider->body->position), dd::colors::Green);
			}
		}

		if (result.isColliding)
		{
			if (glm::isnan(collider->body->position) || glm::isnan(result.other->body->position))
				continue;
			result.main = collider;
			ResolveCollision(result);
		}

	}
}

void tmPhysicsMgr::ResolveCollision(SphereCollider* a, SphereCollider* b, CollisionResult result)
{

	glm::vec3 normal = glm::normalize(b->body->position - a->body->position);
	float penetrationDepth = (a->radius + b->radius) - glm::distance(a->body->position, b->body->position);

	a->body->position -= normal * penetrationDepth / 2.0f;
	b->body->position += normal * penetrationDepth / 2.0f;

	glm::vec3 relativeVelocity = b->body->velocity - a->body->velocity;
	float velocityAlongNormal = glm::dot(relativeVelocity, normal);
	float restitution = 0.5f;

	float impulseMagnitude = -(1 + restitution) * velocityAlongNormal / (1 / a->body->mass + 1 / b->body->mass);
	glm::vec3 impulse = impulseMagnitude * normal;

	a->body->velocity -= impulse / a->body->mass;
	b->body->velocity += impulse / b->body->mass;
}

void tmPhysicsMgr::ResolveCollision(SphereCollider* sphere, BoxCollider* box, CollisionResult collision)
{
	if (!collision.isColliding) return;

	if (box->body->isStatic && sphere->body->isStatic) return;

	// Positional correction
	const float biasFactor = 0.8f;
	float correctedDepth = collision.penetrationDepth * biasFactor;
	glm::vec3 correction = collision.normal * correctedDepth;
	float totalInverseMass = (sphere->body->isStatic ? 0 : sphere->body->invMass) +
		(box->body->isStatic ? 0 : box->body->invMass);

	if (totalInverseMass == 0) return; // No movement for two static bodies

	if (!sphere->body->isStatic) {
		sphere->body->position -= correction * (sphere->body->invMass / totalInverseMass);
	}
	if (!box->body->isStatic) {
		box->body->position += correction * (box->body->invMass / totalInverseMass);
	}

	// Velocity update
	glm::vec3 relativeVelocity = box->body->velocity - sphere->body->velocity;
	float velocityAlongNormal = glm::dot(relativeVelocity, collision.normal);

	if (velocityAlongNormal > 0) return;

	float e = std::min(sphere->body->restitution, box->body->restitution);
	float j = -(1 + e) * velocityAlongNormal;
	j /= totalInverseMass;

	glm::vec3 impulse = j * collision.normal;
	if (!sphere->body->isStatic) {
		sphere->body->velocity -= sphere->body->invMass * impulse;
	}
	if (!box->body->isStatic) {
		box->body->velocity += box->body->invMass * impulse;
	}

	/*
	// Calculate angular velocity
	glm::vec3 contactPoint = sphere->body->position - collision.normal * sphere->radius;
	glm::vec3 ra = contactPoint - sphere->body->position;
	glm::vec3 rb = contactPoint - box->body->position;

	glm::vec3 angularImpulseA = glm::cross(ra, impulse);
	glm::vec3 angularImpulseB = glm::cross(rb, impulse);

	glm::vec3 angAVelo = glm::normalize((sphere->body->invInertiaTensor) * glm::normalize(angularImpulseA));
	glm::vec3 angBVelo = glm::normalize((box->body->invInertiaTensor) * glm::normalize(angularImpulseB));

	if (glm::isnan(angAVelo)) angAVelo = glm::vec3(0.0f);
	if (glm::isnan(angBVelo)) angBVelo = glm::vec3(0.0f);

	if (!sphere->body->isStatic) {
		sphere->body->applyTorque(-angAVelo);
	}
	if (!box->body->isStatic) {
		box->body->applyTorque(angBVelo);
	}
	*/
}

void tmPhysicsMgr::ResolveCollision(BoxCollider* a, BoxCollider* b,CollisionResult collision)
{
	if (collision.isColliding) {
		if (a->body->isStatic && b->body->isStatic) return;

		// Positional correction
		const float biasFactor = 0.2f; // Reduced positional bias factor
		float correctedDepth = (collision.penetrationDepth * biasFactor);
		glm::vec3 correction = collision.normal * correctedDepth;
		float totalInverseMass = (a->body->isStatic ? 0 : a->body->invMass) +
			(b->body->isStatic ? 0 : b->body->invMass);

		if (totalInverseMass == 0) return; // No movement for two static bodies

		if (!a->body->isStatic) {
			a->body->position -= correction * (a->body->invMass / totalInverseMass);
		}
		if (!b->body->isStatic) {
			b->body->position += correction * (b->body->invMass / totalInverseMass);
		}

		// Velocity update
		glm::vec3 relativeVelocity = b->body->velocity - a->body->velocity;
		float velocityAlongNormal = glm::dot(relativeVelocity, collision.normal);

		// Only resolve collision if objects are moving towards each other
		if (velocityAlongNormal > 0) return;

		// Calculate restitution (bounciness)
		float e = std::min(a->body->restitution, b->body->restitution);

		// Calculate impulse scalar
		float j = -(1 + e) * velocityAlongNormal;
		j /= totalInverseMass;

		// Apply impulse
		glm::vec3 impulse = j * collision.normal;
		if (!a->body->isStatic) {
			a->body->velocity -= a->body->invMass * impulse;
		}
		if (!b->body->isStatic) {
			b->body->velocity += b->body->invMass * impulse;
		}

		// Angular velocity update
		glm::vec3 center1 = a->body->position;
		glm::vec3 center2 = b->body->position;
		center1 -= collision.normal * (collision.penetrationDepth * 0.5f);
		center2 += collision.normal * (collision.penetrationDepth * 0.5f);

		glm::vec3 contactPoint = (center1 + center2) * 0.5f;

		glm::vec3 ra = contactPoint - a->body->position;
		glm::vec3 rb = contactPoint - b->body->position;

		glm::vec3 angularImpulseA = glm::cross(ra, impulse);
		glm::vec3 angularImpulseB = glm::cross(rb, impulse);

		glm::vec3 angAVelo = a->body->invInertiaTensor * angularImpulseA;
		glm::vec3 angBVelo = b->body->invInertiaTensor * angularImpulseB;

		if (!a->body->isStatic) {
			a->body->applyTorque(angAVelo);
		}
		if (!b->body->isStatic) {
			b->body->applyTorque(angBVelo);
		}
	}
}

void tmRigidBody::Start()
{
	body = new tmPhysicsBody;
}

void tmRigidBody::Update()
{
	body->position = transform()->position;
	body->rotation = transform()->rotation;
}

void tmRigidBody::LateUpdate()
{
	transform()->position = body->position;
	transform()->rotation = body->rotation;
}

void tmRigidBody::EngineRender()
{
	ImGui::Checkbox("Static", &body->isStatic);
	ImGui::DragFloat("Restitution", &body->restitution);
	ImGui::DragFloat("Mass", &body->mass);

	if (ImGui::Button("Reset Body"))
	{
		transform()->position = vec3(0.0);
		transform()->rotation = vec3(0);
		body->velocity = vec3(0);
		body->angularVelocity = vec3(0);
	}
}

void tmBoxCollider::Update()
{
	if (actor()->GetComponent<tmRigidBody>())
	{
		collider->body = actor()->GetComponent<tmRigidBody>()->body;
		actor()->GetComponent<tmRigidBody>()->body->collider = collider;
	}
	collider->size = size * (transform()->GetGlobalScale()/2.f); 
}

void tmBoxCollider::Start()
{
	collider = new BoxCollider;
}

void tmBoxCollider::EngineRender()
{
	glm::vector_drag("Size", size);
}

void tmSphereCollider::EngineRender()
{
	ImGui::DragFloat("Radiius", &radius, 0.01);
}

void tmSphereCollider::Update()
{
	if (actor()->GetComponent<tmRigidBody>())
	{
		collider->body = actor()->GetComponent<tmRigidBody>()->body;
		actor()->GetComponent<tmRigidBody>()->body->collider = collider;
	}
	collider->radius = radius;
}

void tmSphereCollider::Start()
{
	collider = new SphereCollider;
}
