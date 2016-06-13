#include "Physics.h"

#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include "Gizmos.h"

#include "glm/ext.hpp"
#include "glm/gtc/quaternion.hpp"
#include "RagDoll.h"

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))

void MyCollisionCallBack::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; ++i)
	{
		const PxContactPair& cp = pairs[i];
		// only interested in touches found and lost
		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			printf("Collision detected between: ");
			printf("%s & %s /n", pairHeader.actors[0]->getName(), pairHeader.actors[1]->getName());
		}
	}
}

void MyCollisionCallBack::onTrigger(PxTriggerPair* pairs, PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; ++i)
	{
		PxTriggerPair* pair = pairs + 1;
		PxActor* triggerActor = pair->triggerActor;
		PxActor* otherActor = pair->otherActor;
		printf("%s Entered trigger %s/n", otherActor->getName(), triggerActor->getName());
	}
}

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

	SetUpPhysX();
	SetupVisualDebugger();
	SetupTutorial1();
	SetupCSHTutorial();
	//DIYPhysicsSetup();
	
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

#pragma region Fire Spheres
	fireTimer += dt;
	rocketTimer += dt;
	float mass = 20;

	if (fireTimer > 0.5f && glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		SphereClass* ballz;
		float launchSpeed = 15;
		ballz = new SphereClass( m_camera.getPosition() + m_camera.getForward(), m_camera.getForward() * launchSpeed, 0.5f, 0.9f, vec4(0, 0, 0, 1));
		ballz->m_linearDrag = 0.99f;
		ballz->m_elasticity = 0.9f;
		physicsScene->AddActor(ballz);
		fireTimer = 0;
	}
	boxCounter += dt;
	if (fireTimer > 0.5f && glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS)
	{
		float launchSpeed = 15;
		BoxClass* boxxy = new BoxClass(m_camera.getPosition() + m_camera.getForward(), vec3(2, 2, 2), m_camera.getForward() * launchSpeed, quat(), 0.5f);
		physicsScene->AddActor(boxxy);
		boxxy->m_elasticity = 0.2f;
		fireTimer = 0;
	}

#pragma endregion	

    m_camera.update(1.0f / 60.0f);

	UpdatePhysX(dt);
	//physicsScene->Update(dt);
	//physicsScene->AddGizmos();
	UpdateCSHTutorial();

	//if (m_particleEmitter)
	//{
	//	m_particleEmitter->update(dt);
	//	m_particleEmitter->renderParticles();
	//}

	renderGizmos(m_PhysicsScene);
	dt = (float)glfwGetTime();

    return true;
}

void Physics::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
    Gizmos::draw(m_camera.proj, m_camera.view);

    m_renderer->RenderAndClear(m_camera.view_proj);

    glfwSwapBuffers(m_window);
    glfwPollEvents();
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

	PxSimulationEventCallback* myCollisionCallBack = new MyCollisionCallBack();
	m_PhysicsScene->setSimulationEventCallback(myCollisionCallBack);

	//RagDoll* ragdoll = new RagDoll();
	//PxArticulation* ragDollArticulation;
	//ragDollArticulation = ragdoll->MakeRagDoll(m_Physics, ragdoll->ragDollData, PxTransform(PxVec3(5, 5, 0)), 0.1f, m_PhysicsMaterial);
	//m_PhysicsScene->addArticulation(*ragDollArticulation);

	return m_PhysicsScene;
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

void Physics::UpdatePhysX(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}
	counter += a_deltaTime;
	
	//if (counter >= 2)
	//{
	//	float density = 10;

	//	PxBoxGeometry box(2, 2, 2);
	//	PxTransform transform(PxVec3(0, 50, 0));
	//	PxRigidDynamic* dynamicActor = PxCreateDynamic(*m_Physics, transform, box, *m_PhysicsMaterial, density);
	//	//dynamicActor->addForce(PxVec3(0, 50, 0), PxForceMode::eIMPULSE);
	//	PxRigidBodyExt::updateMassAndInertia(*dynamicActor, density);

	//	m_PhysicsScene->addActor(*dynamicActor);
	//	counter = 0;
	//}

	// Setup "Gun"
	if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS && !firing)
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
		vec3 direction = (-m_camera.world[2].xyz());
		PxVec3 velocity = PxVec3(direction.x, direction.y, direction.z) * muzzleSpeed;
		new_actor->setLinearVelocity(velocity, true);
		m_PhysicsScene->addActor(*new_actor);
		firing = true;
	}

	if (glfwGetKey(m_window, GLFW_KEY_B) != GLFW_PRESS)
		firing = false;

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

	m_PhysicsScene->simulate(a_deltaTime);
	while (m_PhysicsScene->fetchResults() == false)
	{

	}
}

void Physics::DIYPhysicsSetup()
{
	physicsScene = new DIYPhysicScene();
	physicsScene->gravity = vec3(0, -10, 0);
	physicsScene->timeStep = dt;

	plane = new Plane(vec3(0, 1, 0), -0.1f);
	physicsScene->AddActor(plane);

	float ballRadius = 0.09f;
	float mass = 0.08f;

	float damping = 0.25f;
	float springCo = 10.0f;
	
	for (int i = 0; i < 10; i++)
	{
		SphereClass* ball2 = new SphereClass(vec3(i,i * 0.2f + 10, 0), vec3(0), mass, ballRadius, vec4(0, 1, 0, 1));
		physicsScene->AddActor(ball2);
		if (i > 0)
		{
			SpringJoint* spring = new SpringJoint(newBall, ball2, springCo, damping);
			physicsScene->AddActor(spring);
		}
		newBall = ball2;
	}
	newBall->m_physicsType = PhysicsType::STATIC;

#pragma region Cloth test
	/*int width = 15;
	for (int row = 0; row < width; row++)
	{
		for (int col = 0; col < width; col++)
		{
			int index = row * width + col;
			ballList[index] = new SphereClass(vec3(col * 0.5f, 10, 5 + row * 0.5f), vec3(0), mass, ballRadius, vec4(0, 1, 0, 1));
			physicsScene->AddActor(ballList[index]);
		}
	}

	ballList[210]->m_physicsType = PhysicsType::STATIC;
	ballList[210]->m_mass = FLT_MAX;
	ballList[210]->m_color = vec4(1, 0, 0, 1);
	ballList[224]->m_physicsType = PhysicsType::STATIC;
	ballList[224]->m_mass = FLT_MAX;
	ballList[224]->m_color = vec4(1, 0, 0, 1);
	ballList[0]->m_physicsType = PhysicsType::STATIC;
	ballList[0]->m_mass = FLT_MAX;
	ballList[0]->m_color = vec4(1, 0, 0, 1);
	ballList[14]->m_physicsType = PhysicsType::STATIC;
	ballList[14]->m_mass = FLT_MAX;
	ballList[14]->m_color = vec4(1, 0, 0, 1);*/

	//for (int row = 0; row < width; row++)
	//{
	//	for (int col = 0; col < width; col++)
	//	{
	//		int index = row * width + col;
	//		SpringJoint* spring;
	//		//Bending Constraint
	//		//Add springs to east + 2, south + 2, south-east + 2, south-west + 2
	//		if (col < width - 2 && col % 2  == 0)
	//		{
	//			// east
	//			spring = new SpringJoint(ballList[row * width + (col + 2)], ballList[index], springCo, damping);
	//			physicsScene->AddActor(spring);
	//			if (row < width - 2)
	//			{
	//				// south-east
	//				spring = new SpringJoint(ballList[(row + 2) * width + (col + 2)], ballList[index], springCo, damping);
	//				physicsScene->AddActor(spring);
	//			}
	//		}
	//		if (row < width - 2 && row % 2 == 0)
	//		{
	//			// south
	//			spring = new SpringJoint(ballList[(row + 2) * width + col], ballList[index], springCo, damping);
	//			physicsScene->AddActor(spring);
	//			if (col > width - 2)
	//			{
	//				//south-west
	//				spring = new SpringJoint(ballList[(row + 2) * width + (col - 2)], ballList[index], springCo, damping);
	//				physicsScene->AddActor(spring);
	//			}
	//		}

	//		// Structural Constraint
	//		// Add spring to the east (next) and south
	//		if (col < width - 1)
	//		{
	//			spring = new SpringJoint(ballList[row * width + (col + 1)], ballList[index], springCo, damping);
	//			physicsScene->AddActor(spring);
	//			if (row < width - 1)
	//			{
	//				// south-east shear constraint
	//				spring = new SpringJoint(ballList[(row + 1) * width + (col + 1)], ballList[index], springCo, damping);
	//				physicsScene->AddActor(spring);
	//			}
	//		}
	//		if (row < width - 1)
	//		{
	//			spring = new SpringJoint(ballList[(row + 1) * width + col], ballList[index], springCo, damping);
	//			physicsScene->AddActor(spring);
	//			if (col > 0)
	//			{
	//				// south-west shear constraint
	//				spring = new SpringJoint(ballList[(row + 1) * width + (col - 1)], ballList[index], springCo, damping);
	//				physicsScene->AddActor(spring);
	//			}
	//		}
	//	}
	//}


#pragma endregion
}

void Physics::SetupTutorial1()
{
	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*m_Physics, pose, PxPlaneGeometry(), *m_PhysicsMaterial);

	const PxU32 numShapes = plane->getNbShapes();
	// add it to the physX scene
	m_PhysicsScene->addActor(*plane);

#pragma region Fluid dynamics
	//PxBoxGeometry side1(4.5f, 1, 0.5f);
	//PxBoxGeometry side2(0.5f, 1, 4.5f);
	//
	//pose = PxTransform(PxVec3(20.0f, 0.5f, 4.0f));
	//PxRigidStatic* box = PxCreateStatic(*m_Physics, pose, side1, *m_PhysicsMaterial);
	//m_PhysicsScene->addActor(*box);
	////m_physXActors.push_back(box);
	
	//pose = PxTransform(PxVec3(20.0f, 0.5f, -4.0f));
	//box = PxCreateStatic(*m_Physics, pose, side1, *m_PhysicsMaterial);
	//m_PhysicsScene->addActor(*box);
	
	//pose = PxTransform(PxVec3(24.0f, 0.5f, 0));
	//box = PxCreateStatic(*m_Physics, pose, side2, *m_PhysicsMaterial);
	//m_PhysicsScene->addActor(*box);
	
	//pose = PxTransform(PxVec3(16.0f, 0.5f, 0));
	//box = PxCreateStatic(*m_Physics, pose, side2, *m_PhysicsMaterial);
	//m_PhysicsScene->addActor(*box);
	
	//PxParticleFluid* pf;
	
	//// create particle system in PhysX SDX
	//// set immutable properties
	//PxU32 maxParticles = 4000;
	//bool perParticleRestOffSet = false;
	//pf = m_Physics->createParticleFluid(maxParticles, perParticleRestOffSet);
	
	//pf->setViscosity(0.9f);
	//pf->setRestParticleDistance(0.5f);
	//pf->setDynamicFriction(0.1f);
	//pf->setStaticFriction(0.1f);
	//pf->setDamping(0);
	//pf->setParticleMass(0.6f);
	//pf->setRestitution(0);
	//pf->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	//pf->setStiffness(100);
	
	//if (pf)
	//{
	//	m_PhysicsScene->addActor(*pf);
	//	m_particleEmitter = new ParticleFluidEmitter(maxParticles, PxVec3(20, 10, 0), pf, 0.01f);
	//	m_particleEmitter->setStartVelocityRange(-0.001f, -250.0f, -200.0f, 0.001f, -250.0f, 0.001f);
	//}
#pragma endregion
	
	//add a box
	//float density = 10;
	//PxBoxGeometry box(2, 2, 2);
	//PxTransform transform(PxVec3(0, 5, 0));
	//PxRigidDynamic* dynamicActor = PxCreateDynamic(*m_Physics, transform, box, *m_PhysicsMaterial, density);
	
	//add it to the physX scene
	//m_PhysicsScene->addActor(*dynamicActor);

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

			for (int i = 0; i < m_scene.mesh_count; ++i)
			{
				m_renderer->PushMesh(&m_scene.meshes[i], *transform);
			}
		}
	}
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
        Gizmos::addAABBFilled(actor_position, extents, geo_color, &rot);
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
        Gizmos::addSphereFilled(actor_position, geo.radius, 16, 16, geo_color, &rot);
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

