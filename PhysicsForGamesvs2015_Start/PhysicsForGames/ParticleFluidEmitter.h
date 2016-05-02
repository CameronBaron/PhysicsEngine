#pragma once
#include <PxPhysicsAPI.h>
#include "glm/ext.hpp"
using namespace physx;
using namespace std;

//simple struct for our particles

struct FluidParticle
{
	bool active;
	float maxTime;
};


//simple class for particle emitter.  For a real system we would make this a base class and derive different emitters from it by making functions virtual and overloading them.
class ParticleFluidEmitter
{
	
public:
	ParticleFluidEmitter(int _maxParticles,PxVec3 _position,PxParticleFluid* _pf,float _releaseDelay);
	~ParticleFluidEmitter();
	void update(float delta);
	void releaseParticle(int);
	bool tooOld(int);
	void renderParticles();

	void setStartVelocityRange(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
private:
	int getNextFreeParticle();
	bool addPhysXParticle(int particleIndex);

	PxParticleFluid*	m_pf;
	int					m_rows;
	int					m_cols;
	int					m_depth;


	int					m_maxParticles;
	FluidParticle*		m_activeParticles;
	float				m_releaseDelay;
	int					m_numberActiveParticles;

	float				m_time;
	float				m_respawnTime;
	float				m_particleMaxAge;
	PxVec3				m_position;
	int					m_boxWidth;
	int					m_boxHeight;

	PxVec3				m_minVelocity;
	PxVec3				m_maxVelocity;
};