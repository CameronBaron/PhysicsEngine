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
#include "CollisionCallBack.h"

using namespace physx;

struct FilterGroup
{
	enum Enum
	{
		ePLAYER = (1 << 0),
		ePLATFORM = (1 << 1),
		eGROUND = (1 << 2),
		eRAGDOLL = (1 << 3),
	};
};

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
	void SetupCSHTutorial();
	void UpdateCSHTutorial();

	PxRigidActor* AddPhysXBox(const PxTransform a_pose, const PxVec3 a_size, const float a_density, const bool a_trigger);
	PxRigidActor* AddPhysXSphere();
	PxRigidActor* AddPhysXPlane();
	void AddPhysXRagDoll(const PxTransform a_pose);
	void AddFluidSimWithContainer(const PxVec3 a_position);

	Scene m_scene;
	mat4 m_tank_transform;

	void renderGizmos(PxScene* physics_scene);

    Renderer* m_renderer;
    FlyCamera m_camera;
    float m_delta_time;
	float dt;

private:
	enum PhysXActorType
	{
		STATIC,
		DYNAMIC
	};

	struct PhysXRigidActor
	{
	public:
		// Sphere
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, const float a_radius, PhysXActorType a_objType, PxMaterial* a_material, const float a_density = 1)
			: PhysXRigidActor(a_physics, a_pose, &PxSphereGeometry(a_radius), a_objType, a_material, a_density)
		{}
		// Plane
		// Box

		PxRigidActor* GetActor() { return actor; }
		void SetActor(PxRigidActor* a_actor) { actor = a_actor; }
	private:
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, PxGeometry* a_geo, PhysXActorType a_objType, PxMaterial* a_material, const float a_density = 1)
		{
			switch (a_objType)
			{
			case PhysXActorType::STATIC:
				actor = PxCreateStatic(*a_physics, a_pose, *a_geo, *a_material);
				break;
			case PhysXActorType::DYNAMIC:
				actor = PxCreateDynamic(*a_physics, a_pose, *a_geo, *a_material, a_density);
				break;
			default:
				break;
			}

			if (actor == NULL)
			{
				printf("WARNING: ACTOR IS NULL\n");
			}
		}

		PxRigidActor* actor;
	};

	CollisionCallBack* m_collisionCallback;
	PxFoundation* m_PhysicsFoundation;
	PxPhysics* m_Physics;
	PxScene* m_PhysicsScene;
	PxMaterial* m_PhysicsMaterial;
	PxMaterial* m_BoxMaterial;
	PxCooking* m_PhysicsCooker;

	PxDefaultErrorCallback mDefaultErrorCallback;
	ParticleFluidEmitter* m_particleEmitter;
	PxControllerManager* m_ControllerManager;
	PxDefaultAllocator mDefaultAllocatorCallback;

	DIYPhysicScene* physicsScene;
	

	//SphereClass* ballList[15*15];
	bool firing = false;
};

#endif //CAM_PROJ_H_
