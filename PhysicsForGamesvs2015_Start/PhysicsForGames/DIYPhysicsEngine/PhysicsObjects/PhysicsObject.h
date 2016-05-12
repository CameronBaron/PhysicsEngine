//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//	Cameron Baron
//	5/02/16
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#pragma once
#include "../Gizmos.h"

#include <../glm/glm.hpp>
#include <../glm/ext.hpp>
#include <PxPhysicsAPI.h>

using namespace glm;

enum ShapeType
{
	PLANE,
	SPHERE,
	BOX,
	CAPSULE,
	JOINT,
	Last = JOINT
};

enum PhysicsType
{
	DYNAMIC,
	STATIC,
	KINEMATIC
};

class PhysicsObject
{
public:

	void virtual Update(vec3 gravity, float timeStep) = 0;
	void virtual Debug() = 0;
	void virtual MakeGizmo() = 0;
	void virtual ResetPosition() {};

	ShapeType m_shapeID;
	PhysicsType m_physicsType;
	bool m_isAwake = true;

};

