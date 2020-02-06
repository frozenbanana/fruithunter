#pragma once
#include "Entity.hpp"
class Apple : public Entity {
private:
	float m_frameTime;
	int m_nrOfFramePhases;
	int m_currentFramePhase;
	float3 m_startPos;
	float3 m_heightPos;
	float3 m_destinationPos;
	float3 m_nextDestinationPos;

	float m_changeRotation;
	float m_endRotation;

	void setDestination();
	float findRequiredRotation(float3 lookAt);

public:
	Apple();
	void updateAnimated(float dt);
	void setNextDestination(float3 nextDest);
	void lookTo(float3 lookAt) { setRotation(float3(0.f, findRequiredRotation(lookAt), 0.f)); };
};
