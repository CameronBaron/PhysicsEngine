//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//	Cameron Baron
//	5/02/16
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#pragma once

#include <../glm/glm.hpp>
#include <PxPhysicsAPI.h>

using namespace glm;

enum ShapeType
{
	PLANE,
	SPHERE,
	BOX,
	CAPSULE
};

class PhysicsObject
{
public:

	void virtual Update(vec3 gravity, float timeStep) = 0;
	void virtual Debug() = 0;
	void virtual MakeGizmo() = 0;
	void virtual ResetPosition() {};

	ShapeType m_shapeID;
};

