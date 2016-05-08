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

typedef bool(*func)(PhysicsObject*, PhysicsObject*);

vec3 DIYPhysicScene::ProjectileMotionPrediction(vec3 initialPos, vec3 initialVelocity, float time)
{
	vec3 posAtTime(1);
	// posAtTime.x = initialPos.x + time * initialVelocity.y
	posAtTime.x = initialPos.x + time * initialVelocity.x;

	// posAtTime.y = (0.5f * gravity * time * time) + time * initialVelocity.x + initialPos.y
	posAtTime.y = (0.5f * gravity.y * time * time) + time * initialVelocity.y + initialPos.y;

	return posAtTime;
}

// Function pointer array for collisions
static func collisionFunctionArray[] =
{
	DIYPhysicScene::Plane2Plane,	DIYPhysicScene::Plane2Sphere,	DIYPhysicScene::Plane2Box,		DIYPhysicScene::Plane2Capsule,
	DIYPhysicScene::Sphere2Plane,	DIYPhysicScene::Sphere2Sphere,	DIYPhysicScene::Sphere2Box,		DIYPhysicScene::Sphere2Capsule,
	DIYPhysicScene::Box2Plane,		DIYPhysicScene::Box2Sphere,		DIYPhysicScene::Box2Box,		DIYPhysicScene::Box2Capsule,
	DIYPhysicScene::Capsule2Plane,  DIYPhysicScene::Capsule2Sphere, DIYPhysicScene::Capsule2Box,	DIYPhysicScene::Capsule2Capsule,
};

void DIYPhysicScene::CheckForCollision()
{
	int actorCount = actors.size();

	for (int outer = 0; outer < actorCount - 1; outer++)
	{
		for (int inner = outer + 1; inner < actorCount; inner++)
		{
			PhysicsObject* obj1 = actors[outer];
			PhysicsObject* obj2 = actors[inner];
			int _shapeID1 = obj1->m_shapeID;
			int _shapeID2 = obj2->m_shapeID;
			// Using function pointers
			int functionIndex = (_shapeID1 * ShapeType::Last) + _shapeID2;
			func collisionFunctionPtr = collisionFunctionArray[functionIndex];
			if (collisionFunctionPtr != nullptr)
			{
				collisionFunctionPtr(obj1, obj2);
			}
		}
	}
}

bool DIYPhysicScene::Plane2Plane(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Plane2Sphere(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Sphere2Plane with objects swapped
	Sphere2Plane(obj2, obj1);
	return false;
}

bool DIYPhysicScene::Sphere2Plane(PhysicsObject * obj1, PhysicsObject * obj2)
{
	SphereClass* sphere = dynamic_cast<SphereClass*>(obj1);
	Plane* plane = dynamic_cast<Plane*>(obj2);

	if (sphere != nullptr && plane != nullptr)
	{
		vec3 collisionNormal = plane->m_normal;
		float sphereToPlane = glm::dot(sphere->m_position, plane->m_normal) - plane->m_distance;

		if (sphereToPlane < 0) // If we are behind the plane then we flip the normal
		{
			collisionNormal *= -1;
			sphereToPlane *= -1;
		}
		float intersection = sphere->m_radius - sphereToPlane;
		if (intersection > 0)
		{
			// Collision occurs
		}
	}

	return false;
}

bool DIYPhysicScene::Sphere2Sphere(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Try to cast the objects to spheres
	SphereClass* sphere1 = dynamic_cast<SphereClass*>(obj1);
	SphereClass* sphere2 = dynamic_cast<SphereClass*>(obj2);
	//if successful then test for collision
	if (sphere1 != nullptr && sphere2 != nullptr)
	{
		//do collision things here
		// you need code which sets the velocity of the two spheres
		// to zeros if they are overlapping
		vec3 temp = sphere1->m_velocity;
		sphere1->m_velocity += sphere2->m_velocity;
		sphere2->m_velocity += sphere1->m_velocity;
	}
	return false;
}
