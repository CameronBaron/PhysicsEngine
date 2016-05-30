#pragma once
#include "PhysicsObject.h"
#include "RigidBody.h"

class SpringJoint : public PhysicsObject
{
public:
	SpringJoint(RigidBody* connection1, RigidBody* connection2, float springCoefficient, float damping);

private:
	void virtual Update(vec3 gravity, float timeStep);
	void virtual Debug();
	void virtual MakeGizmo();

	RigidBody* m_connections[2];
	float m_damping;
	float m_restLength;
	float m_springCoefficient;
};

