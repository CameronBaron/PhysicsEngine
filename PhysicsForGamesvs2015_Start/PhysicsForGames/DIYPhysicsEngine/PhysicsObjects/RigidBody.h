//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//	Cameron Baron
//	5/02/16
//	Use Unity RigidBody as reference http://docs.unity3d.com/ScriptReference/Rigidbody.html
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#pragma once

#include "PhysicsObject.h"

enum ForceType
{
	IMPULSE,
	ACCELERATION
};

class RigidBody : public PhysicsObject
{
public:
	RigidBody(vec3 position, vec3 velocity, quat rotation, float mass);

	void virtual Update(vec3 gravity, float timeStep);
	void virtual Debug();
	void ApplyForce(vec3 force, ForceType type, float time = 0);
	void ApplyForceToActor(RigidBody* actor2, vec3 force, ForceType type);

	vec3 m_position;
	vec3 m_velocity;
	vec3 m_acceleration;
	float m_mass;
	float m_rotation2D;

	vec4 m_color;
};

