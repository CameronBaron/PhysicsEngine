#include "CollisionCallBack.h"

CollisionCallBack::CollisionCallBack()
{
	m_triggered = false;
}

void CollisionCallBack::onContact(const PxContactPairHeader& a_pairHeader, const PxContactPair* a_pairs, PxU32 a_nbPairs)
{
	for (PxU32 i = 0; i < a_nbPairs; ++i)
	{
		const PxContactPair& cp = a_pairs[i];
		// only interested in touches found and lost
		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			m_triggered = true;
		}
	}
}

void CollisionCallBack::onTrigger(PxTriggerPair* a_pairs, PxU32 a_nbPairs)
{
	for (PxU32 i = 0; i < a_nbPairs; ++i)
	{
		PxTriggerPair* pair = a_pairs + i;
		m_triggered = true;
		m_triggerBody = pair->otherActor;
	}
}