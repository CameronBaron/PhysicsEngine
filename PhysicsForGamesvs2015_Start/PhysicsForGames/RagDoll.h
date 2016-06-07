#pragma once
#include <PxPhysicsAPI.h>

using namespace physx;

enum RagDollParts
{
	NO_PARENT = -1,
	LOWER_SPINE,
	LEFT_PELVIS,
	RIGHT_PELVIS,
	LEFT_UPPER_LEG,
	RIGHT_UPPER_LEG,
	LEFT_LOWER_LEG,
	RIGHT_LOWER_LEG,
	LEFT_FOOT,
	RIGHT_FOOT,
	UPPER_SPINE,
	LEFT_CLAVICLE,
	RIGHT_CLAVICLE,
	NECK,
	HEAD,
	LEFT_UPPER_ARM,
	RIGHT_UPPER_ARM,
	LEFT_LOWER_ARM,
	RIGHT_LOWER_ARM,
	LEFT_HAND,
	RIGHT_HAND,
};

struct RagDollNode
{
	PxQuat globalRotation;
	PxVec3 scaledGlobalPos;
	int parentNodeIdx;			// Index of the parent node
	float halfLength;			// half length og the capsule for this node
	float radius;				// radius of capsule for this node
	float parentLinkPos;		// relative position to the link centre in the parent to this node. 0 is the centre of the node,
								// -1 is left end of the capsule and 1 is right end of the capsule relative to x
	float childLinkPos;			// relative position of link centre in child
	char* name;					// name of link
	PxArticulationLink* linkPtr;

	//constructor
	RagDollNode(PxQuat a_globalRotation, int a_parentNodeID, float a_halfLength, float a_radius, float a_parentLinkPos, float a_childLinkPos, char* a_name)
	{
		globalRotation = a_globalRotation;
		parentNodeIdx = a_parentNodeID;
		halfLength = a_halfLength;
		radius = a_radius;
		parentLinkPos = a_parentLinkPos;
		childLinkPos = a_childLinkPos;
		name = a_name;
	}
};

class RagDoll
{
public:
	RagDoll();
	~RagDoll();

	PxArticulation* MakeRagDoll(PxPhysics* a_physics, RagDollNode** a_nodeArray, PxTransform a_worldPos, float a_scaleFactor, PxMaterial* a_ragdollMaterial);

	const PxVec3 X_AXIS = PxVec3(1, 0, 0);
	const PxVec3 Y_AXIS = PxVec3(0, 1, 0);
	const PxVec3 Z_AXIS = PxVec3(0, 0, 1);

	float jointStiffness = 20;
	float jointDamping = 20;
	float jointSwingLimit = 0.4f;
	float jointTwistLimit = 0.1;

	RagDollNode* ragDollData[21] =
	{
		new RagDollNode(PxQuat(PxPi / 2.0f, Z_AXIS),		NO_PARENT,			1, 3, 1, 1, "lower spine"),
		new RagDollNode(PxQuat(PxPi, Z_AXIS),				LOWER_SPINE,		1, 1, -1, 1, "left pelvis"),
		new RagDollNode(PxQuat(0, Z_AXIS),					LOWER_SPINE,		1, 1, -1, 1, "right pelvis"),
		new RagDollNode(PxQuat(PxPi / 2.0f + 0.2f, Z_AXIS), LEFT_PELVIS,		5, 2, -1, 1, "L upper leg"),
		new RagDollNode(PxQuat(PxPi / 2.0f - 0.2f, Z_AXIS), RIGHT_PELVIS,		5, 2, -1, 1, "R upper leg"),
		new RagDollNode(PxQuat(PxPi / 2.0f + 0.2f, Z_AXIS), LEFT_UPPER_LEG,		5, 1.75f, -1, 1, "L lower leg"),
		new RagDollNode(PxQuat(PxPi / 2.0f - 0.2f, Z_AXIS), RIGHT_UPPER_LEG,	5, 1.75f, -1, 1, "R lower leg"),
		new RagDollNode(PxQuat(0.3f, Z_AXIS),					LEFT_LOWER_LEG,		3, 2, -1, 1, "L foot"),
		new RagDollNode(PxQuat(0.3f, Z_AXIS),					RIGHT_LOWER_LEG,	3, 2, -1, 1, "R foot"),
		new RagDollNode(PxQuat(PxPi / 2.0f, Z_AXIS),		LOWER_SPINE,		1, 3, 1, -1, "upper spine"),
		new RagDollNode(PxQuat(PxPi, Z_AXIS),				UPPER_SPINE,		1, 1.5f, 1, 1, "left clavicle"),
		new RagDollNode(PxQuat(0, Z_AXIS),					UPPER_SPINE,		1, 1.5f, 1, 1, "right clavicle"),
		new RagDollNode(PxQuat(PxPi / 2.0f, Z_AXIS),		UPPER_SPINE,		1, 1, 1, -1, "neck"),
		new RagDollNode(PxQuat(PxPi / 2.0f, Z_AXIS),		NECK,				1, 3, 1, -1, "head"),
		new RagDollNode(PxQuat(PxPi - 0.3f, Z_AXIS),		LEFT_CLAVICLE,		3, 1.5f, -1, 1, "left upper arm"),
		new RagDollNode(PxQuat(0.3f, Z_AXIS),				RIGHT_CLAVICLE,		3, 1.5f, -1, 1, "right upper arm"),
		new RagDollNode(PxQuat(PxPi - 0.3f, Z_AXIS),		LEFT_UPPER_ARM,		3, 1, -1, 1, "left lower arm"),
		new RagDollNode(PxQuat(0.3f, Z_AXIS),				RIGHT_UPPER_ARM,	3, 1, -1, 1, "right lower arm"),
		new RagDollNode(PxQuat(PxPi / 2.0f + 0.2f, Y_AXIS),	LEFT_LOWER_ARM,		1, 1, -1, 0, "L hand"),
		new RagDollNode(PxQuat(PxPi / 2.0f + 0.2f, Y_AXIS),	RIGHT_LOWER_ARM,	1, 1, -1, 0, "R hand"),
		NULL
	};
};

