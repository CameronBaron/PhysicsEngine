#include "DIYPhysicScene.h"



DIYPhysicScene::DIYPhysicScene()
{
}


DIYPhysicScene::~DIYPhysicScene()
{
}

void DIYPhysicScene::AddActor(PhysicsObject * a_actorToAdd)
{
	actors.push_back(a_actorToAdd);
}

void DIYPhysicScene::RemoveActor(PhysicsObject * a_actorToRemove)
{
	auto iter = std::find(actors.begin(), actors.end(), a_actorToRemove);
	if (iter != actors.end())
	{
		actors.erase(iter);
	}
}

void DIYPhysicScene::Update(float dt)
{
	if (dt < timeStep)
		return;
	for (PhysicsObject* po : actors)
	{
		po->Update(gravity, dt);
	}
}

void DIYPhysicScene::AddGizmos()
{
	for (PhysicsObject* po : actors)
	{
		po->MakeGizmo();
	}
}

vec3 DIYPhysicScene::ProjectileMotionPrediction(vec3 initialPos, vec3 initialVelocity, float time)
{
	vec3 posAtTime(1);
	// posAtTime.x = initialPos.x + time * initialVelocity.y
	posAtTime.x = initialPos.x + time * initialVelocity.x;

	// posAtTime.y = (0.5f * gravity * time * time) + time * initialVelocity.x + initialPos.y
	posAtTime.y = (0.5f * gravity.y * time * time) + time * initialVelocity.y + initialPos.y;

	return posAtTime;
}