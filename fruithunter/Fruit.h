#pragma once
#include "Entity.h"

class Fruit : public Entity {
protected:
	int m_nrOfFramePhases; // nr of phases to a movement
	int m_currentFramePhase;
	float m_frameTime;	// the value that is used to interpolate between key frames
	float3 m_startPos;	// Start of movement animation
	float3 m_heightPos; // middle of movement animation
	float3 m_destinationPos;
	float3 m_nextDestinationPos;

	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;

	void setDestination();
	float findRequiredRotation(float3 lookAt);

	Fruit();

public:
	virtual void updateAnimated(float dt) = 0;
	void setNextDestination(float3 nextDest);
	void lookTo(float3 lookAt);
};
