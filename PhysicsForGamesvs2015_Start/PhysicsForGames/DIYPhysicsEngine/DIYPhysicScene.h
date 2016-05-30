#pragma once
#include "PhysicsObjects\SphereClass.h"
#include "PhysicsObjects\Plane.h"
#include "PhysicsObjects\BoxClass.h"
#include "PhysicsObjects\SpringJoint.h"

#include <vector>


class DIYPhysicScene
{
public:
	
	DIYPhysicScene();
	~DIYPhysicScene();

	void AddActor(PhysicsObject* actorToAdd);
	void RemoveActor(PhysicsObject* actorToRemove);
	void Update(float deltaTime);
	void DebugScene();
	void AddGizmos();

	vec3 ProjectileMotionPrediction(vec3 initialPos, vec3 initialVelocity, float time);
	void CheckForCollision();

// CollsionChecks
	static bool Plane2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Plane2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Plane2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Plane2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Plane2Joint(PhysicsObject* obj1, PhysicsObject* obj2);

	static bool Sphere2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Sphere2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Sphere2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Sphere2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Sphere2Joint(PhysicsObject* obj1, PhysicsObject* obj2);

	static bool Box2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Box2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Box2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Box2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Box2Joint(PhysicsObject* obj1, PhysicsObject* obj2);

	static bool Capsule2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Capsule2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Capsule2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Capsule2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Capsule2Joint(PhysicsObject* obj1, PhysicsObject* obj2);

	static bool Joint2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Joint2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Joint2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Joint2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	static bool Joint2Joint(PhysicsObject* obj1, PhysicsObject* obj2);

	static void Response(RigidBody* obj1, RigidBody* obj2, float overlap, vec3 normal);
	static void Seperate(RigidBody* obj1, RigidBody* obj2, float overlap, vec3 normal);

	vec3 gravity;
	float timeStep = 0;
	std::vector<PhysicsObject*> actors;

};

