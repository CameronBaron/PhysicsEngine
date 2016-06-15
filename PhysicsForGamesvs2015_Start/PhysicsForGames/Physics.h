#ifndef SOUND_PROGRAMMING_H_
#define SOUND_PROGRAMMING_H_

#include "Application.h"
#include "Camera.h"
#include "Render.h"
#include "DIYPhysicsEngine/DIYPhysicScene.h"
#include "DIYPhysicsEngine\PhysicsObjects\Plane.h"
#include "DIYPhysicsEngine\PhysicsObjects\SpringJoint.h"
#include "RagDoll.h"
#include "FBXFile.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>
#include <list>
#include "ParticleEmitter.h"
#include "ParticleFluidEmitter.h"
#include "CollisionCallBack.h"
#include "MyControllerHitReport.h"

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

	// DIY Physics
	void DIYPhysicsSetup();
	void DIYPhysicsUpdate(float dt);
	void DIYPhysicsDraw();
	SphereClass* MakeDIYSphere(vec3 a_startPos, float a_mass, float a_radius, vec3 a_initialVel, vec4 a_colour, PhysicsType a_physicsType = PhysicsType::DYNAMIC);
	BoxClass* MakeDIYBox(vec3 a_startPos, float a_mass, vec3 a_size, vec3 a_initialVel, vec4 a_colour, PhysicsType a_physicsType = PhysicsType::DYNAMIC);
	Plane* MakeDIYPlane(vec3 a_normal, float a_offset);
	void MakeDIYString(vec3 a_startPos, unsigned int a_linkCount, float a_linkMass = 0.05f, float a_springCoef = 10.0f, float a_springDamp = 0.25f, bool a_bothEndsStatic = false);
	void MakeDIYCloth(vec3 a_startPos, unsigned int a_width = 15, float a_linkMass = 0.05f, float a_springCoef = 10.0f, float a_springDamp = 0.25f);

	DIYPhysicScene* physicsScene;
	std::vector<SphereClass*> ballList;
    Renderer* m_renderer;
    FlyCamera m_camera;


#pragma region PhysX
	enum PhysXActorType
	{
		STATIC,
		DYNAMIC
	};

	void SetupVisualDebugger();
	PxScene* SetUpPhysX();
	void UpdatePhysX(float deltaTime);
	void SetupScene();
	void SetupCSHTutorial();
	void UpdateCSHTutorial();

	PxRigidActor* AddPhysXBox(const PxTransform a_pose, const PxVec3 a_size, const float a_density, PhysXActorType a_objType, const bool a_trigger = false);
	PxRigidActor* AddPhysXSphere(const PxTransform a_pose, const float a_radius, const float a_density, PhysXActorType a_objType, const bool a_trigger = false);
	PxRigidActor* AddPhysXPlane(const PxTransform a_pose, PhysXActorType a_objType, const bool a_trigger = false);
	PxRigidActor* AddPhysXCapsule(const PxTransform a_pose, float a_radius, float a_halfHeight, const float a_density, PhysXActorType a_objType, const bool a_trigger = false);
	void AddPhysXRagDoll(PxPhysics* a_physics, const PxTransform a_pose, float a_scale, PxMaterial* a_material);
	void AddFluidSimWithContainer(const PxVec3 a_position);
	void CharacterControls(float dt);

	struct PhysXRigidActor
	{
	public:
		// Sphere
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, const float a_radius, PhysXActorType a_objType, PxMaterial* a_material, const float a_density = 1)
			: PhysXRigidActor(a_physics, a_pose, &PxSphereGeometry(a_radius), a_objType, a_material, a_density)
		{}
		// Plane
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, PhysXActorType a_objType, PxMaterial* a_material)
			: PhysXRigidActor(a_physics, a_pose, &PxPlaneGeometry(), a_objType, a_material)
		{}
		// Box
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, const PxVec3 a_size, PhysXActorType a_objType, PxMaterial* a_material, const float a_density = 1)
			: PhysXRigidActor(a_physics, a_pose, &PxBoxGeometry(a_size.x, a_size.y, a_size.z), a_objType, a_material, a_density)
		{}
		// Capsule
		PhysXRigidActor(PxPhysics* a_physics, PxTransform a_pose, const float a_radius, float a_halfHeight, PhysXActorType a_objType, PxMaterial* a_material, const float a_density = 1)
			: PhysXRigidActor(a_physics, a_pose, &PxCapsuleGeometry(a_radius, a_halfHeight), a_objType, a_material, a_density)
		{}

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

	// Player controller vars
	MyControllerHitReport* m_myHitReport;
	PxControllerManager* m_characterManager;
	PxController* m_playerController;
	float m_characterYVelocity;
	float m_characterRotation;
	float m_playerGravity;
	PxExtendedVec3 startingPosition;

	std::vector<PxRigidActor*> m_physXActors;
	std::vector<PxArticulation*> m_physXRagDollActors;

	Scene m_scene;
	mat4 m_tank_transform;

	void renderGizmos(PxScene* physics_scene);


#pragma endregion


	
	
    float m_delta_time;
	float dt;
	bool firingDIY = false;
	bool firingPhysX = false;
	bool updateFluid = false;
};

#endif //CAM_PROJ_H_
