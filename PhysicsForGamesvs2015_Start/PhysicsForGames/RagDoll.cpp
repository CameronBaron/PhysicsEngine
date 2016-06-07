#include "RagDoll.h"


RagDoll::RagDoll()
{

}


RagDoll::~RagDoll()
{
}

PxArticulation* RagDoll::MakeRagDoll(PxPhysics* a_physics, RagDollNode** a_nodeArray, PxTransform a_worldPos, float a_scaleFactor, PxMaterial* a_ragdollMaterial)
{
	// create the articulation for our ragdoll
	PxArticulation* articulation = a_physics->createArticulation();
	RagDollNode** currentNode = a_nodeArray;
	// While there are more nodes to process
	while (*currentNode != NULL)
	{
		//get pointer to the current node
		RagDollNode* currentNodePtr = *currentNode;
		// create a pointer ready to hold the parent node pointer if there is one
		RagDollNode* parentNode = nullptr;
		// get scaled values for capsule
		float radius = currentNodePtr->radius * a_scaleFactor;
		float halfLength = currentNodePtr->halfLength * a_scaleFactor;
		float childHalfLength = radius + halfLength;
		float parentHalfLength = 0; // Will be set later if there is a parent
		// Get pointer to the parent
		PxArticulationLink* parentLinkPtr = NULL;
		currentNodePtr->scaledGlobalPos = a_worldPos.p;

		if (currentNodePtr->parentNodeIdx != NO_PARENT)
		{
			// if there is a parent, then we need to work out our local position for the link
			// get a pointer to the parent node
			parentNode = *(a_nodeArray + currentNodePtr->parentNodeIdx);
			// get a pointer to the link for the parent
			parentLinkPtr = parentNode->linkPtr;
			parentHalfLength = (parentNode->radius + parentNode->halfLength) * a_scaleFactor;
			// work out the local position of the node
			PxVec3 currentRelative = currentNodePtr->childLinkPos * currentNodePtr->globalRotation.rotate(PxVec3(childHalfLength, 0, 0));
			PxVec3 parfentRelative = -currentNodePtr->parentLinkPos * parentNode->globalRotation.rotate(PxVec3(parentHalfLength, 0, 0));
			currentNodePtr->scaledGlobalPos = parentNode->scaledGlobalPos - (parfentRelative + currentRelative);
		}

		// build the transform for the link
		PxTransform linkTransform = PxTransform(currentNodePtr->scaledGlobalPos, currentNodePtr->globalRotation);
		// create the link in the articulation
		PxArticulationLink* link = articulation->createLink(parentLinkPtr, linkTransform);
		// add the pointer to this link into the ragdoll data so we have it for later when we want to link to it
		currentNodePtr->linkPtr = link;
		float jointSpace = 0.01f;	// Gap between joints
		float capsuleHalfLength = (halfLength > jointSpace ? halfLength - jointSpace : 0) + 0.01f;
		PxCapsuleGeometry capsule(radius, capsuleHalfLength);
		link->createShape(capsule, *a_ragdollMaterial);		// Adds a capsule collider to the link
		PxRigidBodyExt::updateMassAndInertia(*link, 50.0f);		// Adds some mass, mass should really be part of the data

		if (currentNodePtr->parentNodeIdx != NO_PARENT)
		{
			// get the pointer to the joint from the link
			PxArticulationJoint* joint = link->getInboundJoint();
			// get the relative rotation of this link
			PxQuat frameRotation = parentNode->globalRotation.getConjugate() * currentNodePtr->globalRotation;
			// set the parent constraint frame
			PxTransform parentConstraintFrame = PxTransform(PxVec3(currentNodePtr->parentLinkPos * parentHalfLength, 0, 0), frameRotation);
			// set the child constraint frame (this is the constraint frome of the newly added link)
			PxTransform thisConstraintFrame = PxTransform(PxVec3(currentNodePtr->childLinkPos * childHalfLength, 0, 0));
			// set up the poses for the joint so it is in the correct place
			joint->setParentPose(parentConstraintFrame);
			joint->setChildPose(thisConstraintFrame);
			// set up some constraints to stop it flopping around
			joint->setStiffness(jointStiffness);
			joint->setDamping(jointDamping);
			joint->setSwingLimit(jointSwingLimit, jointSwingLimit);
			joint->setSwingLimitEnabled(true);
			joint->setTwistLimit(-jointTwistLimit, jointTwistLimit);
			joint->setTwistLimitEnabled(true);
		}

		currentNode++;
	}
	return articulation;
}
