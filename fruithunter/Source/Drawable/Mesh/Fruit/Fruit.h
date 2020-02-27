#pragma once
#include "Entity.h"
#include "AI.h"
#include "TerrainManager.h"

class Fruit : public Entity, public AI {
protected:
	// Phyics based movment
	float3 m_directionalVelocity;
	float3 m_acceleration = float3(0.0f, -400.0f, 0.0f);

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
	// Face
	vector<vector<Material>> m_materials;
	vector<string> m_materialNames;
	// -------------------

	float3 m_worldHome;
	int m_fruitType;

	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;
	void setAnimationDestination();
	float findRequiredRotation(float3 lookAt);
	void enforceOverTerrain();
	Fruit(float3 pos = float3(0.f, 0.f, 0.f));
	void behaviorReleased() override;
	void behaviorInactive(float3 playerPosition) override;


public:
	void release(float3 direction);
	void move(float dt);
	void update(float dt, float3 playerPosition, vector<shared_ptr<Entity>> collidables);
	void handleAvailablePath();
	virtual void updateAnimated(float dt) = 0;
	void jump(float3 direction, float power);
	void setStartPosition(float3 pos);
	void setNextDestination(float3 nextDest);
	void lookTo(float3 lookAt);
	void setWorldHome(float3 pos);
	bool withinDistanceTo(float3 target, float treshhold);
	float3 getHomePosition() const;
	void setVelocity(float3 velo);

	void hit() {
		changeState(CAUGHT);
		m_currentMaterial = 2;
	}

	int getFruitType();
};
