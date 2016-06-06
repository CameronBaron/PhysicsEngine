#include "DIYPhysicScene.h"
#include <iterator>

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
	DIYPhysicScene::Plane2Plane,	DIYPhysicScene::Plane2Sphere,	DIYPhysicScene::Plane2Box,		DIYPhysicScene::Plane2Capsule,	DIYPhysicScene::Plane2Joint,
	DIYPhysicScene::Sphere2Plane,	DIYPhysicScene::Sphere2Sphere,	DIYPhysicScene::Sphere2Box,		DIYPhysicScene::Sphere2Capsule, DIYPhysicScene::Sphere2Joint,
	DIYPhysicScene::Box2Plane,		DIYPhysicScene::Box2Sphere,		DIYPhysicScene::Box2Box,		DIYPhysicScene::Box2Capsule,	DIYPhysicScene::Box2Joint,
	DIYPhysicScene::Capsule2Plane,  DIYPhysicScene::Capsule2Sphere, DIYPhysicScene::Capsule2Box,	DIYPhysicScene::Capsule2Capsule,DIYPhysicScene::Capsule2Joint,
	DIYPhysicScene::Joint2Plane,	DIYPhysicScene::Joint2Sphere,	DIYPhysicScene::Joint2Box,		DIYPhysicScene::Joint2Capsule,	DIYPhysicScene::Joint2Joint
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
	CheckForCollision();
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
	posAtTime.x = initialPos.x + time * initialVelocity.x;
	posAtTime.y = (0.5f * gravity.y * time * time) + time * initialVelocity.y + initialPos.y;

	return posAtTime;
}

void DIYPhysicScene::CheckForCollision()
{
	int actorCount = actors.size();

	// Try sorting actor list to have all springs at the end/beginning and skip over them to save cpu
	// Try using iterator to save some cpu
	
	//for (auto iter = actors.begin(); iter != actors.end() - 1; )
	//{
	//	PhysicsObject* obj1 = (*iter);
	//	//auto iterNext = std::next((iter), 1);
	//	PhysicsObject* obj2 = (*++iter);
	//
	//	if (obj1->m_shapeID == ShapeType::JOINT || obj2->m_shapeID == ShapeType::JOINT)
	//		continue;
	//
	//	if (obj1->m_physicsType == PhysicsType::STATIC && obj2->m_physicsType == PhysicsType::STATIC)
	//		continue;
	//
	//	int _shapeID1 = obj1->m_shapeID;
	//	int _shapeID2 = obj2->m_shapeID;
	//	// Using function pointers
	//	int functionIndex = (_shapeID1 * (ShapeType::Last + 1)) + _shapeID2;
	//	func collisionFunctionPtr = collisionFunctionArray[functionIndex];
	//	if (collisionFunctionPtr != nullptr)
	//	{
	//		collisionFunctionPtr(obj1, obj2);
	//	}
	//}

	// nested for loops may be using more cpu than needed!!!
	for (int outer = 0; outer < actorCount - 1; outer++)
	{
		for (int inner = outer + 1; inner < actorCount; inner++)
		{
			PhysicsObject* obj1 = actors[outer];
			PhysicsObject* obj2 = actors[inner];
	
			if (obj1->m_shapeID == ShapeType::JOINT || obj2->m_shapeID == ShapeType::JOINT)
				continue;
	
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
		vec3 planeNormal = plane->m_normal;
		float planeDist = plane->m_distance;
		vec3 planePos = planeNormal * planeDist;

		vec3 boxPos = box->m_position;

		vec3 corners[8]
		{
			{boxPos + box->min},								 // -,-,-
			{boxPos + vec3(box->min.x, box->min.y, box->max.z)}, // -,-,+
			{boxPos + vec3(box->max.x, box->min.y, box->max.z)}, // +,-,+
			{boxPos + vec3(box->max.x, box->min.y, box->min.z)}, // +,-,-

			{boxPos + box->max},								 // +,+,+
			{boxPos + vec3(box->max.x, box->max.y, box->min.z)}, // +,+,-
			{boxPos + vec3(box->min.x, box->max.y, box->min.z)}, // -,+,-
			{boxPos + vec3(box->min.x, box->max.y, box->max.z)}, // -,+,+
		};

		float closest = glm::dot(boxPos, planeNormal);
		for (vec3 point : corners)
		{
			float pointToPlane = glm::dot(point, planeNormal);
			if (pointToPlane < closest)
				closest = pointToPlane;

			if (closest < planeDist)
			{
				float intersection = planeDist - closest;
				vec3 resultVector = -1 * box->m_mass * planeNormal * glm::dot(planeNormal, box->m_linearVelocity);
				
				vec3 collisionVector = -1.5 * plane->m_normal * dot(box->GetMomentum(), plane->m_normal);

				box->m_position += plane->m_normal * intersection;
				box->AddMomentum(collisionVector);
				return true;
			}
		}
	}

	return false;
}

bool DIYPhysicScene::Plane2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{

	return false;
}

bool DIYPhysicScene::Plane2Joint(PhysicsObject * obj1, PhysicsObject * obj2)
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
			vec3 collisionVector = -1.5 * plane->m_normal * dot(sphere->GetMomentum(), plane->m_normal);

			sphere->m_position += plane->m_normal * intersection;
			sphere->AddMomentum(collisionVector);
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
		float combinedMass = 1.0f / (1.0f / sphere1->m_mass + 1.0f / sphere2->m_mass);
		vec3 forceVector = collisionVector * combinedMass;

		// Collision Elasticity
		float combinedElasticity = (sphere1->m_elasticity + sphere2->m_elasticity) / 2.0f;
		forceVector *= combinedElasticity;

		// Apply resultant vector to objects
		float massRatio1 = sphere1->m_mass / (sphere1->m_mass + sphere2->m_mass);
		float massRatio2 = sphere2->m_mass / (sphere1->m_mass + sphere2->m_mass);

		// Move spheres apart
		Response(sphere2, sphere1, -intersection, glm::normalize(delta));

		return true;
	}
	return false;
}

bool DIYPhysicScene::Sphere2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{
	SphereClass* sphere = dynamic_cast<SphereClass*>(obj1);
	BoxClass* box = dynamic_cast<BoxClass*>(obj2);
	if (sphere == nullptr || box == nullptr)
		return false;

	vec3 distance = sphere->m_position - box->m_position;
	vec3 clampedPoint = distance;

	if (distance.x < box->min.x)
		clampedPoint.x = box->min.x;
	else if (distance.x > box->max.x)
		clampedPoint.x = box->max.x;

	if (distance.y < box->min.y)
		clampedPoint.y = box->min.y;
	else if (distance.y > box->max.y)
		clampedPoint.y = box->max.y;

	if (distance.z < box->min.z)
		clampedPoint.z = box->min.z;
	else if (distance.z > box->max.z)
		clampedPoint.z = box->max.z;

	vec3 clampedDistance = distance - clampedPoint;

	float overlap = glm::length(clampedDistance) - sphere->m_radius;
	if (overlap < 0)
	{
		Response(box, sphere, -overlap, glm::normalize(distance));
		return true;
	}

	return false;
}

bool DIYPhysicScene::Sphere2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Sphere2Joint(PhysicsObject * obj1, PhysicsObject * obj2)
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
		vec3 boxDelta = box2->m_position - box1->m_position;
		vec3 boxExtentsCombined = box1->max + box2->max;

		float xOverlap = std::abs(boxDelta.x) - boxExtentsCombined.x;
		float yOverlap = std::abs(boxDelta.y) - boxExtentsCombined.y;
		float zOverlap = std::abs(boxDelta.z) - boxExtentsCombined.z;

		if (xOverlap <= 0 && yOverlap <= 0 && zOverlap <= 0)
		{
			float minOverlap = xOverlap;
			minOverlap = yOverlap < 0 ? glm::max(minOverlap, yOverlap) : minOverlap;
			minOverlap = zOverlap < 0 ? glm::max(minOverlap, zOverlap) : minOverlap;

			vec3 seperationNormal(0);

			if (xOverlap == minOverlap) seperationNormal.x = std::signbit(boxDelta.x) ? -1.0f : 1.0f;
			if (yOverlap == minOverlap) seperationNormal.y = std::signbit(boxDelta.y) ? -1.0f : 1.0f;
			if (zOverlap == minOverlap) seperationNormal.z = std::signbit(boxDelta.z) ? -1.0f : 1.0f;

			glm::normalize(seperationNormal);

			Response(box1, box2, -minOverlap, seperationNormal);
			return true;
		}
	}

	return false;
}

bool DIYPhysicScene::Box2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Box2Joint(PhysicsObject * obj1, PhysicsObject * obj2)
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

bool DIYPhysicScene::Capsule2Joint(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Joint2Plane(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Joint2Sphere(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Joint2Box(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Joint2Capsule(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

bool DIYPhysicScene::Joint2Joint(PhysicsObject * obj1, PhysicsObject * obj2)
{
	return false;
}

void DIYPhysicScene::Response(RigidBody * obj1, RigidBody * obj2, float overlap, vec3 normal)
{
	Seperate(obj1, obj2, overlap, normal);

	const float coefficientOfRestitution = 0.9f;

	vec3 relativeVel = obj2->m_linearVelocity - obj1->m_linearVelocity;
	float velocityAlongNormal = glm::dot(relativeVel, normal);
	float impulseAmount = -(1 - coefficientOfRestitution) * velocityAlongNormal;
	impulseAmount /= 1 / obj1->m_mass + 1 / obj2->m_mass;

	//float combinedElasticity = (obj1->m_elasticity + obj2->m_elasticity) / 2.0f;
	vec3 impulse = impulseAmount * normal;

	// Apply change in momentum
	if (obj1->m_physicsType != PhysicsType::STATIC)
		obj1->AddVelocity(1 / obj1->m_mass * -impulse);
	if (obj2->m_physicsType != PhysicsType::STATIC)
		obj2->AddVelocity(1 / obj2->m_mass * impulse);
}

void DIYPhysicScene::Seperate(RigidBody * obj1, RigidBody * obj2, float overlap, vec3 normal)
{
	float totalMass = obj1->m_mass + obj2->m_mass;
	float massRatio1 = obj1->m_mass / totalMass;
	float massRatio2 = obj2->m_mass / totalMass;

	// Seperation relative to the objects
	vec3 seperationVector = normal * overlap;
	if (obj1->m_physicsType != PhysicsType::STATIC)
		obj1->m_position += ( -seperationVector * massRatio2);
	if (obj2->m_physicsType != PhysicsType::STATIC)
		obj2->m_position += ( seperationVector * massRatio1);
}
