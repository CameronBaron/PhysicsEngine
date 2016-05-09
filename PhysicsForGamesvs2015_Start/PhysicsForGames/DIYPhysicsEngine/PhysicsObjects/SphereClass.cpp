#include "SphereClass.h"


SphereClass::SphereClass(vec3 position, vec3 velocity, float mass, float radius, vec4 colour) : RigidBody(position, velocity, quat(), mass), m_radius(radius)
{
	m_shapeID = ShapeType::SPHERE;
	m_physicsType = PhysicsType::DYNAMIC;
	m_color = colour;
}

void SphereClass::MakeGizmo()
{
	//Gizmos::add2DCircle(m_position, m_radius, 10, vec4(1, 0, 0, 1));
	Gizmos::addSphere(m_position, m_radius, 10, 10, m_color);
}
