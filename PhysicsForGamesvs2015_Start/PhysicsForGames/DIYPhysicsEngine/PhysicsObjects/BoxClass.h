#pragma once
#include "RigidBody.h"
class BoxClass : public RigidBody
{
public:
	BoxClass(vec3 pos, vec3 bounds, vec3 vel, quat rot, float mass);
	~BoxClass();

	void MakeGizmo();

	vec3 min, max;
	vec3 centreOfGravity;
};

