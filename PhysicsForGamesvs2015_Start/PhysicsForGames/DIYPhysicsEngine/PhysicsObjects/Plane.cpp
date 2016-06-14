#include "Plane.h"

Plane::Plane(vec3 normal, float distance) : m_normal(normal), m_distance(distance)
{
	m_shapeID = ShapeType::PLANE;
	m_physicsType = PhysicsType::STATIC;
}

void Plane::MakeGizmo()
{
	float lineSegmentLength = 300;
	vec3 centrePoint = m_normal * m_distance;
	vec3 parallel = vec3(m_normal.y, -m_normal.x, m_normal.y);
	vec4 color(0, 0, 0, 1);
	vec3 start = centrePoint + (parallel * lineSegmentLength);
	vec3 end = centrePoint - (parallel * lineSegmentLength);
	Gizmos::addLine(start, end, color);
	Gizmos::addAABBFilled(centrePoint, start, color);
}
