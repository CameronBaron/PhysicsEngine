#include "BoxClass.h"

BoxClass::BoxClass(vec3 pos, vec3 extents, vec3 vel, quat rot, float mass) : RigidBody(pos, vel, rot, mass)
{
	min = -extents * 0.5f;
	max = extents * 0.5f;
	m_physicsType = PhysicsType::DYNAMIC;
	m_shapeID = ShapeType::BOX;
}

BoxClass::~BoxClass()
{
}

void BoxClass::MakeGizmo()
{
	Gizmos::addAABBFilled(m_position, max, vec4(0, 1, 0, 1));
}
