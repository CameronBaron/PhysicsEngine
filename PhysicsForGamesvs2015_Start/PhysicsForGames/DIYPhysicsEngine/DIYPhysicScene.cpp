#include "DIYPhysicScene.h"



static bool operator < (const glm::vec3 &lhs, const glm::vec3 &rhs)
{
	if (lhs.x < rhs.x &&
		lhs.y < rhs.y &&
		lhs.z < rhs.z)
		return true;
	return false;
}
static bool operator <= (const vec3 &lhs, const vec3 &rhs)
{
	if (lhs.x <= rhs.x &&
		lhs.y <= rhs.y &&
		lhs.z <= rhs.z)
		return true;
	return false;
}
static bool operator > (const vec3 &lhs, const vec3 &rhs)
{
	if (lhs.x > rhs.x &&
		lhs.y > rhs.y &&
		lhs.z > rhs.z)
		return true;
	return false;
}
static bool operator >= (const vec3 &lhs, const vec3 &rhs)
{
	if (lhs.x >= rhs.x &&
		lhs.y >= rhs.y &&
		lhs.z >= rhs.z)
		return true;
	return false;
}

typedef bool(*func)(PhysicsObject*, PhysicsObject*);
// Function pointer array for collisions
static func collisionFunctionArray[] =
{
	DIYPhysicScene::Plane2Plane,	DIYPhysicScene::Plane2Sphere,	DIYPhysicScene::Plane2Box,		DIYPhysicScene::Plane2Capsule,
	DIYPhysicScene::Sphere2Plane,	DIYPhysicScene::Sphere2Sphere,	DIYPhysicScene::Sphere2Box,		DIYPhysicScene::Sphere2Capsule,
	DIYPhysicScene::Box2Plane,		DIYPhysicScene::Box2Sphere,		DIYPhysicScene::Box2Box,		DIYPhysicScene::Box2Capsule,
	DIYPhysicScene::Capsule2Plane,  DIYPhysicScene::Capsule2Sphere, DIYPhysicScene::Capsule2Box,	DIYPhysicScene::Capsule2Capsule
};

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
	for (PhysicsObject* po : actors)
	{
		po->Update(gravity, dt);
	}
	CheckForCollision();
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
	posAtTime.x = initialPos.x + time * initialVelocity.x;
	posAtTime.y = (0.5f * gravity.y * time * time) + time * initialVelocity.y + initialPos.y;

	return posAtTime;
}

void DIYPhysicScene::CheckForCollision()
{
	int actorCount = actors.size();

	for (int outer = 0; outer < actorCount - 1; outer++)
	{
		for (int inner = outer + 1; inner < actorCount; inner++)
		{
			PhysicsObject* obj1 = actors[outer];
			PhysicsObject* obj2 = actors[inner];

			if (obj1->m_physicsType == PhysicsType::STATIC && obj2->m_physicsType == PhysicsType::STATIC)
				continue;

			int _shapeID1 = obj1->m_shapeID;
			int _shapeID2 = obj2->m_shapeID;
			// Using function pointers
			int functionIndex = (_shapeID1 * (ShapeType::Last + 1)) + _shapeID2;
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
	return Sphere2Plane(obj2, obj1);
}

bool DIYPhysicScene::Plane2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{
	Plane* plane = dynamic_cast<Plane*>(obj1);
	BoxClass* box = dynamic_cast<BoxClass*>(obj2);
	if (plane != nullptr && box != nullptr)
	{
		// Check for collision here
	}

	return false;
}

bool DIYPhysicScene::Plane2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{

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

		float intersection = sphere->m_radius - sphereToPlane;
		if (intersection > 0)
		{
			// Collision occurs
			vec3 planeNormal = plane->m_normal;

			vec3 resultVector = -1 * sphere->m_mass * planeNormal * (dot(planeNormal, sphere->m_linearVelocity));
			//sphere->ApplyForce(resultVector, ForceType::ACCELERATION);
			sphere->m_position += planeNormal * intersection;
			return true;
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
	if (sphere1 == nullptr || sphere2 == nullptr)
	return false;

	vec3 delta = sphere2->m_position - sphere1->m_position;
	float distance = glm::length(delta);
	float intersection = sphere1->m_radius + sphere2->m_radius - distance;
	// check collision here
	if (intersection > 0)
	{
		// Find the point where the collision occured
		// The plane is static, so collision response only applies to sphere
		vec3 collisionNormal = normalize(delta);
		vec3 relativeVelocity = sphere2->m_linearVelocity - sphere1->m_linearVelocity;
		vec3 collisionVector = collisionNormal * (dot(relativeVelocity, collisionNormal));
		vec3 forceVector = collisionVector * 1.0f / (1.0f / sphere1->m_mass + 1.0f / sphere2->m_mass);

		float massRatio1 = sphere1->m_mass / (sphere1->m_mass + sphere2->m_mass);
		float massRatio2 = sphere2->m_mass / (sphere1->m_mass + sphere2->m_mass);
		sphere1->m_linearVelocity = forceVector * massRatio2 * 0.5f;
		sphere2->m_linearVelocity = -forceVector * massRatio1 * 0.5f;

		//sphere1->ApplyForceToActor(sphere2, 2 * forceVector, ForceType::ACCELERATION);

		//sphere1->ApplyForce(forceVector, ForceType::ACCELERATION);

		vec3 seperationVector = collisionNormal * intersection * 0.5f;
		if (sphere1->m_physicsType == PhysicsType::DYNAMIC)
			sphere1->m_position -= seperationVector * massRatio2;
		if (sphere2->m_physicsType == PhysicsType::DYNAMIC)
			sphere2->m_position += seperationVector * massRatio1;



		return true;
	}
	return false;
}

bool DIYPhysicScene::Sphere2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{

	return false;
}

bool DIYPhysicScene::Sphere2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{

	return false;
}

bool DIYPhysicScene::Box2Plane(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Plane2Box
	return Plane2Box(obj2, obj1);
}

bool DIYPhysicScene::Box2Sphere(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Sphere2Box
	return Sphere2Box(obj2, obj1);
}

bool DIYPhysicScene::Box2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{
	BoxClass* box1 = dynamic_cast<BoxClass*>(obj1);
	BoxClass* box2 = dynamic_cast<BoxClass*>(obj2);
	if (box1 != nullptr && box2 != nullptr)
	{
		if (box1->max > box2->min && box1->min < box2->min)
		{
			// Rotate and check points?

			// Collision occurs
		}
	}

	return false;
}

bool DIYPhysicScene::Box2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Capsule2Plane(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Plane2Capsule
	return Plane2Capsule(obj2, obj1);
}

bool DIYPhysicScene::Capsule2Sphere(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Sphere2Capsule
	return Sphere2Capsule(obj2, obj1);
}

bool DIYPhysicScene::Capsule2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{
	// Use Box2Capsule
	return Box2Capsule(obj2, obj1);
}

bool DIYPhysicScene::Capsule2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}
