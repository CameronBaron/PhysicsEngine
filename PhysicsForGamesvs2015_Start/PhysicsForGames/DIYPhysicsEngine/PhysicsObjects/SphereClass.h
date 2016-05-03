#pragma once
#include "RigidBody.h"


class SphereClass : public RigidBody
{
public:
	SphereClass(vec3 position, vec3 velocity, float mass, float radius, vec4 colour);

	void virtual MakeGizmo();

	float m_radius;
};

