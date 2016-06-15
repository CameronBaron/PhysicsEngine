#pragma once
#include <PxPhysicsAPI.h>

using namespace physx;

class MyControllerHitReport : public PxUserControllerHitReport
{
public:
	MyControllerHitReport() : PxUserControllerHitReport() {};

	virtual void onShapeHit(const PxControllerShapeHit &hit)
	{
		PxRigidActor* actor = hit.shape->getActor();

		_playerContactNormal = hit.worldNormal;

		PxRigidDynamic* myActor = actor->is<PxRigidDynamic>();
		if (myActor)
		{
			// this is where we can apply forcesto things we hit
		}
	};

	virtual void onControllerHit(const PxControllersHit &hit) {};
	virtual void onObstacleHit(const PxControllerObstacleHit &hit) {};
	PxVec3 getPlayerContactNormal() { return _playerContactNormal; }
	void clearPlayerContactNormal() { _playerContactNormal = PxVec3(0, 0, 0); };

	PxVec3 _playerContactNormal;
};

