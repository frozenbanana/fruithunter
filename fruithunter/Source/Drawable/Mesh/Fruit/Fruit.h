#pragma once
#include "Entity.h"
#include "AI.h"

class Fruit : public Entity, public AI {
protected:
	int m_nrOfFramePhases; // nr of phases to a movement
	int m_currentFramePhase;
	float m_frameTime;	// the value that is used to interpolate between key frames
	float3 m_startPos;	// Start of movement animation
	float3 m_heightPos; // middle of movement animation
	float3 m_destinationPos;
	float3 m_nextDestinationPos;
	int m_fruitType;

	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;

	void setDestination();
	float findRequiredRotation(float3 lookAt);

	Fruit(float3 pos = float3(0.f, 0.f, 0.f));

public:
	virtual void update(float dt, float3 playerPos) = 0;
	virtual void updateAnimated(float dt) = 0;
	void setStartPosition(float3 pos);
	void setNextDestination(float3 nextDest);
	void lookTo(float3 lookAt);

	int getFruitType();
};
