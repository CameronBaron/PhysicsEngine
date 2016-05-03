#include "RigidBody.h"

RigidBody::RigidBody(vec3 a_position, vec3 a_velocity, quat a_rotation, float a_mass) :
	m_position(a_position), m_velocity(a_velocity), m_mass(a_mass)
{
	m_rotation2D = 0;
}

void RigidBody::Update(vec3 a_gravity, float a_timeStep)
{	
	// Add gravity to velocity
	m_velocity += (a_gravity * a_timeStep);
	
	// Add velocity to position
	m_position += (m_velocity * a_timeStep);
}

void RigidBody::Debug()
{
}

void RigidBody::ApplyForce(vec3 a_force, ForceType type)
{
	switch (type)
	{
	case ACCELERATION: {m_velocity += (a_force / m_mass); break; }
	case IMPULSE: {m_velocity += a_force; break; }
	}
}

void RigidBody::ApplyForceToActor(RigidBody *actor2, vec3 force, ForceType type)
{
	actor2->ApplyForce(force, type);
	ApplyForce(-force, type);
}
