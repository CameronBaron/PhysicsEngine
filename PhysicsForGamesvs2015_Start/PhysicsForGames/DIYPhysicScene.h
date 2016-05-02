#pragma once
#include "PhysicsObjects\PhysicsObject.h"

#include <vector>

class DIYPhysicScene
{
public:
	DIYPhysicScene();
	~DIYPhysicScene();

	void AddActor(PhysicsObject* actorToAdd);
	void RemoveActor(PhysicsObject* actorToRemove);
	void Update();
	void DebugScene();
	void AddGizmos();

	vec3 gravity;
	float timeStep;
	std::vector<PhysicsObject*> actors;
};

