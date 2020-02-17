#pragma once
#include "Entity.h"
#include "AI.h"
#include "..\Terrain\TerrainManager.h"

class Fruit : public Entity, public AI {
protected:
	// Phyics based movment
	float3 m_direction;
	float3 m_velocity;
	// -------------------
	int m_nrOfFramePhases; // nr of phases to a movement
	int m_currentFramePhase;
	float m_frameTime; // the value that is used to interpolate between key frames
	// Animation
	float3 m_startAnimationPosition;  // Start of movement animation
	float3 m_heightAnimationPosition; // middle of movement animation
	float3 m_destinationAnimationPosition;
	float3 m_nextDestinationAnimationPosition;
	// -------------------
	float3 m_worldHome;


	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;
	void setDestination();
	float findRequiredRotation(float3 lookAt);

	Fruit(float3 pos = float3(0.f, 0.f, 0.f));

public:
	virtual void updateAnimated(float dt) = 0;
	void setStartPosition(float3 pos);
	void setNextDestination(float3 nextDest);
	void lookTo(float3 lookAt);
	void setWorldHome(float3 pos);

	void hit() { changeState(CAUGHT); }
};
