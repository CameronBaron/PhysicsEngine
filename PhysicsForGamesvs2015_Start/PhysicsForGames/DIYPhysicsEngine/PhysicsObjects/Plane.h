#pragma once
#include "PhysicsObject.h"

class Plane : public PhysicsObject
{
public:
	Plane(vec3 normal, float distance);
	void virtual Update(vec3 gravity, float timeStep) {};
	void virtual Debug() {};
	void virtual MakeGizmo();

	vec3 m_normal;
	float m_distance;

};

