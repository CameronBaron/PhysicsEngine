#include "BoxClass.h"



BoxClass::BoxClass(vec3 pos, vec3 extents, vec3 vel, quat rot, float mass) : RigidBody(pos, vel, rot, mass)
{
	min = -extents * 0.5f;
	max = extents * 0.5f;
}


BoxClass::~BoxClass()
{
}
