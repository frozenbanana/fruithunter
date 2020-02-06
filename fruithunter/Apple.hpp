#pragma once
#include "Entity.h"
class Apple : public Entity {
private:
	float m_frameTime;
	int m_nrOfFramePhases;
	int m_currentFramePhase;
	float3 m_startPos;
	float3 m_heightPos;
	float3 m_destinationPos;
	float3 m_nextDestinationPos;



	void setDestination();

public:
	Apple();
	void updateAnimated(float dt);
	void setNextDestination(float3 nextDest);
};
