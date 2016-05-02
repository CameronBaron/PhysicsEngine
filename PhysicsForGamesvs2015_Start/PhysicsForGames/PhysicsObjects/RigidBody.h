//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//	Cameron Baron
//	5/02/16
//	Use Unity RigidBody as reference http://docs.unity3d.com/ScriptReference/Rigidbody.html
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#pragma once

#include "PhysicsObject.h"


class RigidBody : public PhysicsObject
{
public:
	RigidBody(vec3 position, vec3 velocity, quat rotation, float mass);

	void virtual Update(vec2 gravity, float timeStep);
	void virtual Debug();
	void ApplyForce(vec2 force);
	void ApplyForceToActor(RigidBody* actor2, vec3 force);

	vec2 m_position;
	vec2 m_velocity;
	float m_mass;
	float m_rotation2D;
};

