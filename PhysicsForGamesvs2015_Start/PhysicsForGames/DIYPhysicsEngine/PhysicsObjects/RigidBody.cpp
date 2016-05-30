#include "RigidBody.h"
#include <limits>

#define MIN_LINEAR_THRESHOLD 0.01f
#define MAX_LINEAR_THRESHOLD 100.0f

RigidBody::RigidBody(vec3 a_position, vec3 a_velocity, quat a_rotation, float a_mass) :
	m_position(a_position), m_linearVelocity(a_velocity), m_mass(a_mass)
{
	m_rotation = vec3(0);
	m_linearDrag = 1;
	m_rotationalDrag = 1;
	m_rotationalInertia = 1;
	m_staticFriction = 1;
	m_dynamicFriction = 1;
	m_bounciness = 1;
	m_elasticity = 1;

	if (m_physicsType == PhysicsType::STATIC)
	{
		a_mass = std::numeric_limits<float>::max();
	}
}

void RigidBody::Update(vec3 a_gravity, float a_deltaTime)
{
	if (m_isAwake)
	{
		// Add gravity to acceleration
		if (m_physicsType != PhysicsType::STATIC)
			m_acceleration += a_gravity;
		// Add acceleration to velocity
		m_linearVelocity += m_acceleration * a_deltaTime;

		// Apply Drag
		m_linearVelocity *= m_linearDrag;

		if (glm::length(m_linearVelocity) < MIN_LINEAR_THRESHOLD)
		{
			m_linearVelocity = vec3(0);
		}
		else if (glm::length(m_linearVelocity) > MAX_LINEAR_THRESHOLD)
		{
			m_linearVelocity *= m_rotationalDrag;
		}

		// Add velocity to position
		m_position += (m_linearVelocity * a_deltaTime);
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

void RigidBody::ApplyForceToActor(RigidBody *a_actor2, vec3 a_force, ForceType a_type)
{
	ApplyForce(a_force, a_type);
	a_actor2->ApplyForce(-a_force, a_type);
}

void RigidBody::ApplyTorque(float a_torque, vec3 a_direction)
{
	m_angularVelocity += (a_torque * a_direction) / m_mass;
}

void RigidBody::AddVelocity(vec3 velocity)
{
	m_linearVelocity += velocity;
}

void RigidBody::AddMomentum(vec3 momentum)
{
	AddVelocity(momentum / m_mass);
}

vec3 RigidBody::GetMomentum()
{
	return m_linearVelocity * m_mass;
}

