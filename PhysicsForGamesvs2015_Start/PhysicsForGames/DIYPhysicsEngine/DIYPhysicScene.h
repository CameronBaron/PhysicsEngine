#pragma once
#include "PhysicsObjects\SphereClass.h"
#include "PhysicsObjects\Plane.h"

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
	bool Plane2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Plane2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Plane2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Plane2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Sphere2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Sphere2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Sphere2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Sphere2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Box2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Box2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Box2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Box2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Capsule2Plane(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Capsule2Sphere(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Capsule2Box(PhysicsObject* obj1, PhysicsObject* obj2);
	bool Capsule2Capsule(PhysicsObject* obj1, PhysicsObject* obj2);

	vec3 gravity;
	float timeStep;
	std::vector<PhysicsObject*> actors;
};

