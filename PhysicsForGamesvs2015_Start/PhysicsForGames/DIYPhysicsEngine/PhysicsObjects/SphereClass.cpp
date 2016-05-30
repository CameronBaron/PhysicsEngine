#include "SphereClass.h"


SphereClass::SphereClass(vec3 position, vec3 velocity, float mass, float radius, vec4 colour) : RigidBody(position, velocity, quat(), mass), m_radius(radius)
{
	m_shapeID = ShapeType::SPHERE;
	m_physicsType = PhysicsType::DYNAMIC;
	m_color = colour;
	m_linearDrag = 0.99f;
	m_elasticity = 0.3f;
}

void SphereClass::MakeGizmo()
{
	//Gizmos::addSphere(m_position, m_radius, 5, 5, m_color);
	Gizmos::addSphereFilled(m_position, m_radius, 14, 14, m_color);
}
