#include "SpringJoint.h"

SpringJoint::SpringJoint(RigidBody * a_connection1, RigidBody * a_connection2, float a_springCoefficient, float a_damping)
{
	m_connections[0] = a_connection1;
	m_connections[1] = a_connection2;
	m_springCoefficient = a_springCoefficient;
	m_damping = a_damping;
	m_restLength = glm::length(m_connections[0]->m_position - m_connections[1]->m_position);
	m_shapeID = ShapeType::JOINT;
}

void SpringJoint::Update(vec3 gravity, float timeStep)
{
	// Using Hooke's Law
	// F = -kX - bv
	// k = spring coefficient
	float k = m_springCoefficient;
	// X = spring displacement
	float X = glm::length(m_connections[0]->m_position - m_connections[1]->m_position) - m_restLength;
	// b = spring damping
	float b = m_damping;
	// v = relative velocity
	float v = glm::length(m_connections[0]->m_linearVelocity - m_connections[1]->m_linearVelocity);

	float force = (-k * X) - (b * v);
	vec3 forceDir = glm::normalize(m_connections[0]->m_position - m_connections[1]->m_position);

	if (m_connections[0]->m_physicsType == PhysicsType::DYNAMIC)
		m_connections[0]->ApplyForce(force * forceDir, ForceType::ACCELERATION);
	if (m_connections[1]->m_physicsType == PhysicsType::DYNAMIC)
		m_connections[1]->ApplyForce(-force * forceDir, ForceType::ACCELERATION);
	Gizmos::addLine(m_connections[0]->m_position, m_connections[1]->m_position, vec4(0, 1, 0, 1));
}

void SpringJoint::Debug()
{
}

void SpringJoint::MakeGizmo()
{
}
