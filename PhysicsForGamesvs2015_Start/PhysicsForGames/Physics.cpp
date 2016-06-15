#include "Physics.h"

#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include "Gizmos.h"

#include "glm/ext.hpp"
#include "glm/gtc/quaternion.hpp"
#include "RagDoll.h"

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))
#define GREEN glm::vec4(0,1,0,1)

bool Physics::startup()
{
    if (Application::startup() == false)
    {
        return false;
    }
	
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    Gizmos::create();
	dt = 0;

    m_camera = FlyCamera(1280.0f / 720.0f, 10.0f);
    m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
    m_camera.sensitivity = 3;

	m_renderer = new Renderer();

	// Make 2 large (physX) trigger boxes, while the camera is inside one
	// either DIY or PhysX will update and opposite for the other

	SetUpPhysX();
	//SetupVisualDebugger();
	DIYPhysicsSetup();
	
    return true;
}

void Physics::shutdown()
{
	m_PhysicsScene->release();
	m_Physics->release();
	m_PhysicsFoundation->release();

	delete m_renderer;
    Gizmos::destroy();
    Application::shutdown();
}

bool Physics::update()
{
    if (Application::update() == false)
    {
        return false;
    }

    Gizmos::clear();

    dt = (float)glfwGetTime() - dt;

    vec4 white(1);
    vec4 black(0, 0, 0, 1);

    for (int i = 0; i <= 20; ++i)
    {
        Gizmos::addLine(vec3(-10 + i, -0.01, -10), vec3(-10 + i, -0.01, 10),
            i == 10 ? white : black);
        Gizmos::addLine(vec3(-10, -0.01, -10 + i), vec3(10, -0.01, -10 + i),
            i == 10 ? white : black);
    }

    m_camera.update(1.0f / 60.0f);

	UpdatePhysX(dt);
	renderGizmos(m_PhysicsScene);

	DIYPhysicsUpdate(dt);

	dt = (float)glfwGetTime();

    return true;
}

void Physics::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
    Gizmos::draw(m_camera.proj, m_camera.view);


	physicsScene->AddGizmos();
    m_renderer->RenderAndClear(m_camera.view_proj);


    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

//~~~~~~~~~~~~~~~~ DIY PHYSICS ~~~~~~~~~~~~~~~~~~//

void Physics::DIYPhysicsSetup()
{
	physicsScene = new DIYPhysicScene();
	physicsScene->gravity = vec3(0, -10, 0);
	physicsScene->timeStep = dt;

	MakeDIYPlane(vec3(0, 1, 0), -0.1f);

	MakeDIYString(vec3(-10,20,-10), 10, 0.08f, 10.0f, 0.9f, false);

#pragma region DIY Scene

	// Left wall
	MakeDIYBox(vec3(-20, 5, 0), 100.0f, vec3(0.2f, 10, 40), vec3(0), GREEN, PhysicsType::STATIC);
	// Back wall
	MakeDIYBox(vec3(-10, 5, -20), 100.0f, vec3(20, 10, 0.2f), vec3(0), GREEN, PhysicsType::STATIC);

	// box stack
	for (int i = 0; i < 5; ++i)
	{
		MakeDIYBox(vec3(-5, 2 + 1 * i, 0), 100.0f, vec3(1, 1, 1), vec3(0), GREEN);
		if (i == 4)
		{
			MakeDIYSphere(vec3(-5, 2 + 2 * i, 0), 1.0f, 1.0f, vec3(0), GREEN);
		}
	}


#pragma endregion

}

void Physics::DIYPhysicsUpdate(float dt)
{
#pragma region Fire Spheres
	float mass = 20;

	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS && !firingDIY)
	{
		float launchSpeed = 15;
		SphereClass* ball = MakeDIYSphere(m_camera.getPosition() + m_camera.getForward(), 5.0f, 0.4f, m_camera.getForward() * launchSpeed, vec4(0, 0, 0, 1));
		ball->m_linearDrag = 0.99f;
		ball->m_elasticity = 0.9f;
		firingDIY = true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		firingDIY = false;
	}

	physicsScene->Update(dt);
	physicsScene->AddGizmos();

#pragma endregion	
}

void Physics::DIYPhysicsDraw()
{
}

SphereClass* Physics::MakeDIYSphere(vec3 a_startPos, float a_mass, float a_radius, vec3 a_initialVel, vec4 a_colour, PhysicsType a_physicsType)
{
	SphereClass* ball = new SphereClass(a_startPos, a_initialVel, a_mass, a_radius, a_colour);
	ball->m_physicsType = a_physicsType;
	physicsScene->AddActor(ball);
	return ball;
}

Plane* Physics::MakeDIYPlane(vec3 a_normal, float a_offset)
{
	Plane* plane = new Plane(a_normal, a_offset);
	physicsScene->AddActor(plane);
	return plane;
}

BoxClass* Physics::MakeDIYBox(vec3 a_startPos, float a_mass, vec3 a_size, vec3 a_initialVel, vec4 a_colour, PhysicsType a_physicsType)
{
	BoxClass* box = new BoxClass(a_startPos, a_size, a_initialVel, quat(), a_mass);
	box->m_physicsType = a_physicsType;
	physicsScene->AddActor(box);
	return box;
}

void Physics::MakeDIYString(vec3 a_startPos, unsigned int a_linkCount, float a_linkMass, float a_springCoef, float a_springDamp, bool a_bothEndsStatic)
{
	float ballRadius = 0.09f;

	SphereClass* ball2;
	for (unsigned int i = 0; i < a_linkCount; i++)
	{
		SphereClass* ball1 = new SphereClass(a_startPos + vec3(i, i * 0.2f, 0), vec3(0), a_linkMass, ballRadius, vec4(0, 1, 0, 1));
		physicsScene->AddActor(ball1);
		if (i > 0)
		{
			SpringJoint* spring = new SpringJoint(ball2, ball1, a_springCoef, a_springDamp);
			physicsScene->AddActor(spring);
		}
		if (i == 0)
		{
			ball1->m_physicsType = PhysicsType::STATIC;
		}
		ball2 = ball1;
	}
	if (a_bothEndsStatic)
		ball2->m_physicsType = PhysicsType::STATIC;
}

void Physics::MakeDIYCloth(vec3 a_startPos, unsigned int a_width, float a_linkMass, float a_springCoef, float a_springDamp)
{
	ballList = std::vector<SphereClass*>();
	float ballRadius = 0.09f;
	int width = 15;
	SphereClass* sp;
	for (int row = 0; row < width; row++)
	{
		for (int col = 0; col < width; col++)
		{
			int index = row * width + col;
			sp = new SphereClass(a_startPos + vec3(col * 0.5f, 10, 5 + row * 0.5f), vec3(0), a_linkMass, ballRadius, vec4(0, 1, 0, 1));
			ballList.push_back(sp);
			physicsScene->AddActor(sp);
		}
	}

	ballList[a_width * a_width - a_width]->m_physicsType = PhysicsType::STATIC;
	ballList[a_width * a_width - a_width]->m_mass = FLT_MAX;
	ballList[a_width * a_width - a_width]->m_color = vec4(1, 0, 0, 1);
	ballList[a_width * a_width -1]->m_physicsType = PhysicsType::STATIC;
	ballList[a_width * a_width - 1]->m_mass = FLT_MAX;
	ballList[a_width * a_width - 1]->m_color = vec4(1, 0, 0, 1);
	ballList[0]->m_physicsType = PhysicsType::STATIC;
	ballList[0]->m_mass = FLT_MAX;
	ballList[0]->m_color = vec4(1, 0, 0, 1);
	ballList[a_width -1]->m_physicsType = PhysicsType::STATIC;
	ballList[a_width - 1]->m_mass = FLT_MAX;
	ballList[a_width - 1]->m_color = vec4(1, 0, 0, 1);

	for (int row = 0; row < width; row++)
	{
		for (int col = 0; col < width; col++)
		{
			int index = row * width + col;
			SpringJoint* spring;
			//Bending Constraint
			//Add springs to east + 2, south + 2, south-east + 2, south-west + 2
			//if (col < width - 2 && col % 2  == 0)
			//{
			//	// east
			//	spring = new SpringJoint(ballList[row * width + (col + 2)], ballList[index], a_springCoef, a_springDamp);
			//	physicsScene->AddActor(spring);
			//	if (row < width - 2)
			//	{
			//		// south-east
			//		spring = new SpringJoint(ballList[(row + 2) * width + (col + 2)], ballList[index], a_springCoef, a_springDamp);
			//		physicsScene->AddActor(spring);
			//	}
			//}
			//if (row < width - 2 && row % 2 == 0)
			//{
			//	// south
			//	spring = new SpringJoint(ballList[(row + 2) * width + col], ballList[index], a_springCoef, a_springDamp);
			//	physicsScene->AddActor(spring);
			//	if (col > width - 2)
			//	{
			//		//south-west
			//		spring = new SpringJoint(ballList[(row + 2) * width + (col - 2)], ballList[index], a_springCoef, a_springDamp);
			//		physicsScene->AddActor(spring);
			//	}
			//}

			// Structural Constraint
			// Add spring to the east (next) and south
			if (col < width - 1)
			{
				spring = new SpringJoint(ballList[row * width + (col + 1)], ballList[index], a_springCoef, a_springDamp);
				physicsScene->AddActor(spring);
				if (row < width - 1)
				{
					// south-east shear constraint
					spring = new SpringJoint(ballList[(row + 1) * width + (col + 1)], ballList[index], a_springCoef, a_springDamp);
					physicsScene->AddActor(spring);
				}
			}
			if (row < width - 1)
			{
				spring = new SpringJoint(ballList[(row + 1) * width + col], ballList[index], a_springCoef, a_springDamp);
				physicsScene->AddActor(spring);
				if (col > 0)
				{
					// south-west shear constraint
					spring = new SpringJoint(ballList[(row + 1) * width + (col - 1)], ballList[index], a_springCoef, a_springDamp);
					physicsScene->AddActor(spring);
				}
			}
		}
	}
}

//~~~~~~~~~~~~~~~~~~~ PHYSX ~~~~~~~~~~~~~~~~~~~~~//

PxFilterFlags myFilterShader(PxFilterObjectAttributes attrib0, PxFilterData filterData0, PxFilterObjectAttributes attrib1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attrib0) || PxFilterObjectIsTrigger(attrib1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	// trigger the contact for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) & (filterData1.word0 & filterData0.word1))
	{
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST;
	}
	return PxFilterFlag::eDEFAULT;
}

void SetupFiltering(PxRigidActor* a_actor, PxU32 a_filterGroup, PxU32 a_filterMask)
{
	PxFilterData filterData;
	filterData.word0 = a_filterGroup;
	filterData.word1 = a_filterMask;

	const PxU32 numShapes = a_actor->getNbShapes();
	PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numShapes, 16);
	a_actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; ++i)
	{
		PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}
	_aligned_free(shapes);
}

void SetShapeAsTrigger(PxRigidActor* a_actor)
{
	PxRigidStatic* staticActor = a_actor->is<PxRigidStatic>();
	assert(staticActor);

	const PxU32 numShapes = staticActor->getNbShapes();
	PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numShapes, 16);
	staticActor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; ++i)
	{
		shapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shapes[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}
}

void Physics::SetupVisualDebugger()
{
	// Check if PVDConnection manager is available on this platform
	if (m_Physics->getPvdConnectionManager() == NULL)
		return;
	// Setup connection parameters
	const char* pvd_host_ip = "127.0.0.1";
	// IP of the PC which is running PVD
	int port = 5425;
	// TCP port to connect to, where PVD is listening
	unsigned int timeout = 100;
	// timeout in milliseconds to wait for PVD to respond,
	// consoles and remote PCs need a higher timeout.
	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	// and now try to connectPxVisualDebuggerExt
	auto theConnection = PxVisualDebuggerExt::createConnection(m_Physics->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectionFlags);
}

PxScene* Physics::SetUpPhysX()
{
	PxAllocatorCallback *myCallback = new myAllocator(); // pointer to memory manager
	m_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, mDefaultErrorCallback); // PXfoundation singleton
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_PhysicsFoundation, PxTolerancesScale()); // Physics system using foundation
	PxInitExtensions(*m_Physics); // Init extension library
	// Create cooker object, used to create complex mes and cloth colliders
	m_PhysicsCooker = PxCreateCooking(PX_PHYSICS_VERSION, *m_PhysicsFoundation, PxCookingParams(PxTolerancesScale()));

	//create physics material // Three parameters: Static Friction, Dynamic Friction, Restitution
	// Restitution controls how "bouncy" an object is.
	m_PhysicsMaterial = m_Physics->createMaterial(1, 1, 0);
	// Create a scene description object: contains the parameters which control the scene
	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	// Set gravity
	sceneDesc.gravity = PxVec3(0, -9.81f, 0);
	// Callback function allows us to catch triggered events(enter trigger zones or collisions)
	sceneDesc.filterShader = myFilterShader;
	// Tells PhysX we are using the CPU for PhysX calcs. (Can use GPU or multiple CPU cores)
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);
	// Create our PhysX scene
	m_PhysicsScene = m_Physics->createScene(sceneDesc);

	m_collisionCallback = new CollisionCallBack();
	m_PhysicsScene->setSimulationEventCallback(m_collisionCallback);

	SetupScene();

	return m_PhysicsScene;
}

void Physics::UpdatePhysX(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}

	CharacterControls(a_deltaTime);

	// Setup "Gun"
	if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS && !firingPhysX)
	{
		vec3 cam_pos = m_camera.world[3].xyz();
		vec3 box_vel = -m_camera.world[2].xyz() * 20.0f;
		PxTransform box_transform(PxVec3(cam_pos.x, cam_pos.y, cam_pos.z));
		// Geometry
		PxSphereGeometry sphere(0.5f);
		// Density
		float density = 50;
		float muzzleSpeed = 20;

		PxRigidDynamic* new_actor = PxCreateDynamic(*m_Physics, box_transform, sphere, *m_PhysicsMaterial, density);
		SetupFiltering(new_actor, FilterGroup::ePLATFORM, FilterGroup::ePLAYER);
		vec3 direction = (-m_camera.world[2].xyz());
		PxVec3 velocity = PxVec3(direction.x, direction.y, direction.z) * muzzleSpeed;
		new_actor->setLinearVelocity(velocity, true);
		new_actor->setName("bullet");
		m_PhysicsScene->addActor(*new_actor);
		firingPhysX = true;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_RELEASE)
		firingPhysX = false;

	// Ragdoll
	//if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
	//{
	//	vec3 cam_pos = m_camera.world[3].xyz();
	//	vec3 box_vel = -m_camera.world[2].xyz() * 20.0f;
	//	PxTransform transform(PxVec3(cam_pos.x, cam_pos.y, cam_pos.z));
	//
	//	RagDoll* ragdoll = new RagDoll();
	//	PxArticulation* ragDollArticulation;
	//	ragDollArticulation = ragdoll->MakeRagDoll(m_Physics, ragdoll->ragDollData, transform, 0.1f, m_PhysicsMaterial);
	//	m_PhysicsScene->addArticulation(*ragDollArticulation);
	//}

	if (m_collisionCallback)
	{
		if (m_collisionCallback->GetTriggered())
		{
			PxRigidActor* triggerActor = m_collisionCallback->GetTriggerBody();

			PxRigidBody* body = triggerActor->isRigidBody();

			if (body)
			{
				body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
			}

			/*const PxU32 numShapes = triggerActor->getNbShapes();
			PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numShapes, 16);
			triggerActor->getShapes(shapes, numShapes);
			for (PxU32 i = 0; i < numShapes; i++)
			{
				shapes[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
				shapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			}*/

			if (m_particleEmitter)
			{
				m_particleEmitter->update(dt);
			}
			
		}
	}

	if (m_particleEmitter)
	{
		m_particleEmitter->renderParticles();
	}

	m_PhysicsScene->simulate(a_deltaTime);
	while (m_PhysicsScene->fetchResults() == false)
	{

	}
}

void Physics::SetupScene()
{
	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*m_Physics, pose, PxPlaneGeometry(), *m_PhysicsMaterial);

	const PxU32 numShapes = plane->getNbShapes();
	// add it to the physX scene
	m_PhysicsScene->addActor(*plane);

	// Back wall
	PxBoxGeometry box(10, 5, 0.2f);
	PxTransform transform(PxVec3(10, 5, -20));
	PxRigidStatic* staticActor = PxCreateStatic(*m_Physics, transform, box, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*staticActor);

	// Right wall
	PxBoxGeometry box2(0.2f, 5, 20);
	PxTransform transform2(PxVec3(20, 5, 0));
	PxRigidStatic* staticActor2 = PxCreateStatic(*m_Physics, transform2, box2, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*staticActor2);

	// Ragdoll
	AddPhysXRagDoll(m_Physics, PxTransform(PxVec3(10, 10, -10)), 0.1f, m_PhysicsMaterial);

	// trigger volume
	PxRigidActor* triggerVol;
	triggerVol = AddPhysXBox(PxTransform(PxVec3(10, 4, 20)), PxVec3(4, 4, 4), 1, PhysXActorType::STATIC, true);
	triggerVol->setName("FluidTrigger");

	// fluid particles
	AddFluidSimWithContainer(PxVec3(10, 1, 10));

	// player controller
	m_myHitReport = new MyControllerHitReport();
	m_characterManager = PxCreateControllerManager(*m_PhysicsScene);
	// Describe our controller
	PxCapsuleControllerDesc desc;
	desc.height = 1.6f;
	desc.radius = 0.4f;
	desc.position.set(0, 0, 0);
	desc.material = m_PhysicsMaterial;
	desc.reportCallback = m_myHitReport;
	desc.density = 10;
	// Create the layer controller
	m_playerController = m_characterManager->createController(desc);
	startingPosition = PxExtendedVec3(0, 0, 0);
	m_playerController->setPosition(startingPosition);
	// setup some controls to control our player with
	m_characterYVelocity = 0;
	m_characterRotation = 0;
	m_playerGravity = -0.5f;
	m_myHitReport->clearPlayerContactNormal();

	AddPhysXCapsule(PxTransform(PxVec3(0, 20, 0)), 1, 2, 1, PhysXActorType::DYNAMIC);
}

void Physics::SetupCSHTutorial()
{
	m_scene = LoadSceneFromOBJ("./data/tank/", "battle_tank.obj");

	PxTransform relativePose;
	PxBoxGeometry box;

	// tank base
	box = PxBoxGeometry(1, 1, 2);

	PxTransform transform(*(PxMat44*)(&m_tank_transform[0]));
	PxRigidDynamic* tank_actor = PxCreateDynamic(*m_Physics, transform, box, *m_PhysicsMaterial, 10);

	int numberShapes = tank_actor->getNbShapes();
	PxShape* shapes;
	tank_actor->getShapes(&shapes, numberShapes);
	relativePose = PxTransform(PxVec3(0, 1, 0));	
	shapes->setGeometry(box);
	shapes->setLocalPose(relativePose);

	// turret


	tank_actor->userData = &box;
	m_PhysicsScene->addActor(*tank_actor);

	
}

void Physics::UpdateCSHTutorial()
{
	PxActorTypeFlags flags = PxActorTypeFlag::eRIGID_DYNAMIC;
	int actor_count = m_PhysicsScene->getNbActors(flags);

	for (int i = 0; i < actor_count; ++i)
	{
		PxActor* actor;
		m_PhysicsScene->getActors(flags, &actor, 1, i);

		if (actor->userData)
		{
			PxRigidActor* rigid_actor = (PxRigidActor*)actor;
			PxMat44 m = rigid_actor->getGlobalPose();
			mat4* transform = (mat4*)actor->userData;
			*transform = *(mat4*)&m;

			for (unsigned int i = 0; i < m_scene.mesh_count; ++i)
			{
				m_renderer->PushMesh(&m_scene.meshes[i], *transform);
			}
		}
	}
}

PxRigidActor* Physics::AddPhysXBox(const PxTransform a_pose, const PxVec3 a_size, const float a_density, PhysXActorType a_objType, const bool a_trigger)
{
	PhysXRigidActor* boxActor = new PhysXRigidActor(m_Physics, a_pose, a_size, a_objType, m_PhysicsMaterial, a_density);

	if (a_trigger && boxActor != NULL)
	{
		SetShapeAsTrigger(boxActor->GetActor());
		SetupFiltering(boxActor->GetActor(), FilterGroup::ePLATFORM, FilterGroup::ePLAYER);
		boxActor->GetActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	m_PhysicsScene->addActor(*boxActor->GetActor());
	return boxActor->GetActor();
}

PxRigidActor* Physics::AddPhysXSphere(const PxTransform a_pose, const float a_radius, const float a_density, PhysXActorType a_objType, const bool a_trigger)
{
	PhysXRigidActor* sphereActor = new PhysXRigidActor(m_Physics, a_pose, a_radius, a_objType, m_PhysicsMaterial, a_density);

	if (a_trigger && sphereActor != NULL)
	{
		SetShapeAsTrigger(sphereActor->GetActor());
		SetupFiltering(sphereActor->GetActor(), FilterGroup::ePLATFORM, FilterGroup::ePLAYER);
		sphereActor->GetActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	m_PhysicsScene->addActor(*sphereActor->GetActor());
	return sphereActor->GetActor();
}

PxRigidActor* Physics::AddPhysXPlane(const PxTransform a_pose, PhysXActorType a_objType, const bool a_trigger)
{
	PhysXRigidActor* planeActor = new PhysXRigidActor(m_Physics, a_pose, a_objType, m_PhysicsMaterial);
	m_PhysicsScene->addActor(*planeActor->GetActor());

	return planeActor->GetActor();
}

PxRigidActor* Physics::AddPhysXCapsule(const PxTransform a_pose, float a_radius, float a_halfHeight, const float a_density, PhysXActorType a_objType, const bool a_trigger)
{
	PhysXRigidActor* capsuleActor = new PhysXRigidActor(m_Physics, a_pose, a_radius, a_halfHeight, a_objType, m_PhysicsMaterial, a_density);

	if (a_trigger && capsuleActor != NULL)
	{
		SetShapeAsTrigger(capsuleActor->GetActor());
		SetupFiltering(capsuleActor->GetActor(), FilterGroup::ePLAYER, FilterGroup::ePLATFORM);
		capsuleActor->GetActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	m_PhysicsScene->addActor(*capsuleActor->GetActor());
	return capsuleActor->GetActor();
}

void Physics::AddPhysXRagDoll(PxPhysics* a_physics, const PxTransform a_pose, float a_scale, PxMaterial* a_material)
{
	RagDoll* ragdoll = new RagDoll();
	PxArticulation* ragDollArticulation;
	ragDollArticulation = ragdoll->MakeRagDoll(a_physics, ragdoll->ragDollData, a_pose, a_scale, a_material);
	m_PhysicsScene->addArticulation(*ragDollArticulation);
	ragDollArticulation->putToSleep();
}

void Physics::AddFluidSimWithContainer(const PxVec3 a_position)
{
	PxBoxGeometry side1(4.0f, 5, 0.1f);
	PxBoxGeometry side2(0.1f, 5, 4.0f);
	
	PxTransform pose = PxTransform(a_position + PxVec3(0, 2.5f, 4.0f));
	PxRigidStatic* box = PxCreateStatic(*m_Physics, pose, side1, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*box);
	
	pose = PxTransform(a_position + PxVec3(0, 2.5f, -4.0f));
	box = PxCreateStatic(*m_Physics, pose, side1, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*box);
	
	pose = PxTransform(a_position + PxVec3(4.0f, 2.5f, 0));
	box = PxCreateStatic(*m_Physics, pose, side2, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*box);
	
	pose = PxTransform(a_position + PxVec3(-4.0f, 2.5f, 0));
	box = PxCreateStatic(*m_Physics, pose, side2, *m_PhysicsMaterial);
	m_PhysicsScene->addActor(*box);
	
	PxParticleFluid* pf;
	
	// create particle system in PhysX SDX
	// set immutable properties
	PxU32 maxParticles = 40000;
	bool perParticleRestOffSet = false;
	pf = m_Physics->createParticleFluid(maxParticles, perParticleRestOffSet);
	
	pf->setViscosity(0.9f);
	pf->setRestParticleDistance(0.4f);
	pf->setDynamicFriction(0.01f);
	pf->setStaticFriction(0.1f);
	pf->setDamping(0.1f);
	pf->setParticleMass(10.0f);
	pf->setRestitution(0);
	pf->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	pf->setStiffness(50);
	
	if (pf)
	{
		m_PhysicsScene->addActor(*pf);
		m_particleEmitter = new ParticleFluidEmitter(maxParticles, a_position + PxVec3(0, 10, 0), pf, 0.05f);
		m_particleEmitter->setStartVelocityRange(-10.0f, 100.0f, -10.0f, 10.0f, 200.0f, 10.0f);
	}
}

void Physics::CharacterControls(float dt)
{
	bool onGround;
	float movementSpeed = 10.0f;
	float rotationSpeed = 10.0f;

	if (m_myHitReport->getPlayerContactNormal().y > 0.1f)
	{
		m_characterYVelocity = -0.1f;
		onGround = true;
	}
	else
	{
		m_characterYVelocity += m_playerGravity * dt;
		onGround = false;
	}
	m_myHitReport->clearPlayerContactNormal();
	const PxVec3 up(0, 1, 0);

	PxVec3 velocity(0, m_characterYVelocity, 0);
	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		velocity.x -= movementSpeed * dt;
	}
	if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		velocity.x += movementSpeed * dt;
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		m_characterRotation -= rotationSpeed * dt;
	}
	if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		m_characterRotation += rotationSpeed * dt;
	}

	float minDistance = 0.001f;
	PxControllerFilters filter;
	PxQuat rotation(m_characterRotation, up);
	m_playerController->move(rotation.rotate(velocity), minDistance, dt, filter);
}

void AddWidget(PxShape* shape, PxRigidActor* actor, vec4 geo_color)
{
    PxTransform full_transform = PxShapeExt::getGlobalPose(*shape, *actor);
    vec3 actor_position(full_transform.p.x, full_transform.p.y, full_transform.p.z);
    glm::quat actor_rotation(full_transform.q.w,
        full_transform.q.x,
        full_transform.q.y,
        full_transform.q.z);
    glm::mat4 rot(actor_rotation);

    mat4 rotate_matrix = glm::rotate(10.f, glm::vec3(7, 7, 7));

    PxGeometryType::Enum geo_type = shape->getGeometryType();

    switch (geo_type)
    {
    case (PxGeometryType::eBOX) :
    {
        PxBoxGeometry geo;
        shape->getBoxGeometry(geo);
        vec3 extents(geo.halfExtents.x, geo.halfExtents.y, geo.halfExtents.z);
        Gizmos::addAABB(actor_position, extents, geo_color, &rot);
    } break;
    case (PxGeometryType::eCAPSULE) :
    {
        PxCapsuleGeometry geo;
        shape->getCapsuleGeometry(geo);
		Gizmos::addCapsule(actor_position, geo.halfHeight * 2, geo.radius, 16, 16, geo_color, &rot);
    } break;
    case (PxGeometryType::eSPHERE) :
    {
        PxSphereGeometry geo;
        shape->getSphereGeometry(geo);
        Gizmos::addSphere(actor_position, geo.radius, 16, 16, geo_color, &rot);
    } break;
    case (PxGeometryType::ePLANE) :
    {

    } break;
    }
}

void Physics::renderGizmos(PxScene* physics_scene)
{
    PxActorTypeFlags desiredTypes = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
    PxU32 actor_count = physics_scene->getNbActors(desiredTypes);
    PxActor** actor_list = new PxActor*[actor_count];
	physics_scene->getActors(desiredTypes, actor_list, actor_count);
    
    vec4 geo_color(1, 0, 0, 1);
    for (int actor_index = 0;
        actor_index < (int)actor_count;
        ++actor_index)
    {
        PxActor* curr_actor = actor_list[actor_index];
        if (curr_actor->isRigidActor())
        {
            PxRigidActor* rigid_actor = (PxRigidActor*)curr_actor;
            PxU32 shape_count = rigid_actor->getNbShapes();
            PxShape** shapes = new PxShape*[shape_count];
            rigid_actor->getShapes(shapes, shape_count);

            for (int shape_index = 0;
                shape_index < (int)shape_count;
                ++shape_index)
            {
                PxShape* curr_shape = shapes[shape_index];
                AddWidget(curr_shape, rigid_actor, geo_color);
            }

            delete[]shapes;
        }
    }

    delete[] actor_list;

    int articulation_count = physics_scene->getNbArticulations();

    for (int a = 0; a < articulation_count; ++a)
    {
        PxArticulation* articulation;
		physics_scene->getArticulations(&articulation, 1, a);

        int link_count = articulation->getNbLinks();

        PxArticulationLink** links = new PxArticulationLink*[link_count];
        articulation->getLinks(links, link_count);

        for (int l = 0; l < link_count; ++l)
        {
            PxArticulationLink* link = links[l];
            int shape_count = link->getNbShapes();

            for (int s = 0; s < shape_count; ++s)
            {
                PxShape* shape;
                link->getShapes(&shape, 1, s);
                AddWidget(shape, link, geo_color);
            }
        }
        delete[] links;
    }
}
