#ifndef SOUND_PROGRAMMING_H_
#define SOUND_PROGRAMMING_H_

#include "Application.h"
#include "Camera.h"
#include "Render.h"
#include "DIYPhysicsEngine/DIYPhysicScene.h"
#include "DIYPhysicsEngine\PhysicsObjects\Plane.h"
#include "DIYPhysicsEngine\PhysicsObjects\SpringJoint.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>
#include <list>
#include "ParticleEmitter.h"
#include "ParticleFluidEmitter.h"

using namespace physx;

class myAllocator : public PxAllocatorCallback
{
public:
	virtual ~myAllocator() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* pointer = _aligned_malloc(size, 16);
		return pointer;
	}
	virtual void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}
};


class Physics : public Application
{
public:
	virtual bool startup();
	virtual void shutdown();
    virtual bool update();
    virtual void draw();

	PxScene* SetUpPhysX();
	void SetupVisualDebugger();
	void UpdatePhysX(float deltaTime);

	void DIYPhysicsSetup();

	void SetupTutorial1();

	void renderGizmos(PxScene* physics_scene);

    Renderer* m_renderer;
    FlyCamera m_camera;
    float m_delta_time;
	float dt;

private:
	PxFoundation* m_PhysicsFoundation;
	PxPhysics* m_Physics;
	PxScene* m_PhysicsScene;
	PxDefaultErrorCallback mDefaultErrorCallback;
	PxDefaultAllocator mDefaultAllocatorCallback;
	PxSimulationFilterShader mDefaultFilterShader = PxDefaultSimulationFilterShader;
	PxMaterial* m_PhysicsMaterial;
	PxMaterial* m_BoxMaterial;
	PxCooking* m_PhysicsCooker;
	PxControllerManager* m_ControllerManager;

	ParticleFluidEmitter* m_particleEmitter;

	DIYPhysicScene* physicsScene;

	float counter = 0;

	SphereClass* newBall;
	SphereClass* newBall2;
	Plane* plane;
	float rocketTimer = 0;
	float fireTimer = 0;
	float boxTimer = 2;
	float boxCounter = 0;

	SpringJoint* joint;

	//SphereClass* ballList[15*15];
	bool firing = false;
};

#endif //CAM_PROJ_H_
