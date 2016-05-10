#include "RigidBody.h"
#include <limits>

RigidBody::RigidBody(vec3 a_position, vec3 a_velocity, quat a_rotation, float a_mass) :
	m_position(a_position), m_linearVelocity(a_velocity), m_mass(a_mass)
{
	m_rotation2D = 0;
	if (m_physicsType == PhysicsType::STATIC)
	{
		a_mass = std::numeric_limits<float>::max();
	}
}

void RigidBody::Update(vec3 a_gravity, float a_timeStep)
{
	if (m_isAwake)
	{
		// Add gravity to acceleration
		if (m_physicsType != PhysicsType::STATIC)
			m_acceleration += a_gravity;
		// Add acceleration to velocity
		m_linearVelocity += m_acceleration * a_timeStep;

		// Apply Drag
		m_linearVelocity *= m_linearDrag;

		// Add velocity to position
		m_position += (m_linearVelocity * a_timeStep);
		m_acceleration = vec3(0);
	}
}

void RigidBody::Debug()
{
}

void RigidBody::ApplyForce(vec3 a_force, ForceType type, float a_time)
{
	switch (type)
	{
	//case IMPULSE: { m_velocity += a_force; break; }
	case ACCELERATION: { m_acceleration += (a_force / m_mass); break; }
	}
}

void RigidBody::ApplyForceToActor(RigidBody *actor2, vec3 force, ForceType type)
{
	actor2->ApplyForce(force, type);
	ApplyForce(-force, type);
}
