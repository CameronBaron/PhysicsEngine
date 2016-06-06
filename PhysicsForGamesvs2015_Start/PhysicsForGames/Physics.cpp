#include "Physics.h"

#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include "Gizmos.h"

#include "glm/ext.hpp"
#include "glm/gtc/quaternion.hpp"
#include <PxPhysicsAPI.h>

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))

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
	renderGizmos(m_PhysicsScene);

	physicsScene->Update(dt);
	physicsScene->AddGizmos();

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
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	// Tells PhysX we are using the CPU for PhysX calcs. (Can use GPU or multiple CPU cores)
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);
	// Create our PhysX scene
	m_PhysicsScene = m_Physics->createScene(sceneDesc);

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
	
	if (counter >= 2)
	{
		float density = 10;

		PxBoxGeometry box(2, 2, 2);
		PxTransform transform(PxVec3(0, 50, 0));
		PxRigidDynamic* dynamicActor = PxCreateDynamic(*m_Physics, transform, box, *m_PhysicsMaterial, density);
		//dynamicActor->addForce(PxVec3(0, 50, 0), PxForceMode::eIMPULSE);
		PxRigidBodyExt::updateMassAndInertia(*dynamicActor, density);

		m_PhysicsScene->addActor(*dynamicActor);
		counter = 0;
	}

	// Setup "Gun"
	if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS)
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
	}

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

	// add it to the physX scene
	m_PhysicsScene->addActor(*plane);

	//add a box
	//float density = 10;
	//PxBoxGeometry box(2, 2, 2);
	//PxTransform transform(PxVec3(0, 5, 0));
	//PxRigidDynamic* dynamicActor = PxCreateDynamic(*m_Physics, transform, box, *m_PhysicsMaterial, density);
	
	//add it to the physX scene
	//m_PhysicsScene->addActor(*dynamicActor);

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

