#include "SphereClass.h"


SphereClass::SphereClass(vec3 position, vec3 velocity, float mass, float radius, vec4 colour) : RigidBody(position, velocity, glm::quat(1), mass)
{
}

void SphereClass::MakeGizmo()
{
}
