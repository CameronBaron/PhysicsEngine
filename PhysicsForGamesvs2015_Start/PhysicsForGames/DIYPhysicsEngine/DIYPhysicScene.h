#pragma once
#include "PhysicsObjects\SphereClass.h"

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

	vec3 gravity;
	float timeStep;
	std::vector<PhysicsObject*> actors;
};

