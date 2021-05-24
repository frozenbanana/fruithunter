#pragma once
#include "Entity.h"
#include "AI.h"
#include "ParticleSystem.h"
#include "TerrainBatch.h"

#define THROWVELOCITY 30.f

class Fruit : public Entity, public AI {
protected:
	// Phyics based movement
	bool m_isVisible = true;
	float3 m_velocity = float3(0.f);
	float m_speed = 0.0f;
	float3 m_gravity = float3(0.0f, -1.0f, 0.0f) * 15.0f; // same as player
	float3 m_direction;
	float m_groundFriction = 10.f;
	float m_airFriction = 5.f;

	bool m_respawning = false;
	float m_startScale = 0.5;
	float m_respawn_timer = 0;
	float m_respawn_timeMax = 1;

	bool m_onGround = true;

	void checkOnGroundStatus();

	float m_passive_speed;
	float m_active_speed;
	float m_caught_speed;
	ParticleSystem m_particleSystem;
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

	Environment* m_boundTerrain = nullptr;
	float3 m_worldHome;
	size_t m_nrOfTriesGoHome = 0;
	FruitType m_fruitType;
	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;
	void setAnimationDestination();
	void enforceOverTerrain();
	Fruit(float3 pos = float3(0.f, 0.f, 0.f));
	void behaviorReleased() override;
	void behaviorInactive() override;

	virtual void setDirection();


	bool m_afterRealease = false;

	float m_aboveGroundThreshold = 0.2f;

	void updateVelocity(float dt);
	void stopMovement();

	void respawn();
	void updateRespawn();

	/* Intersects with Terrain and collidable Entitites. */
	bool rayCastWorld(float3 point, float3 forward, float3& intersection, float3& normal);
	/* Checks Terrain and Entity distance from feet and returns true if distance is less than specified threshold. */
	bool isOnGround(float3 position, float heightThreshold);

public:
	bool isVisible() const;
	virtual void release(float3 direction);
	void move(float dt);
	virtual void update();
	virtual void updateAnimated(float dt) = 0;
	virtual void draw_fruit();
	virtual void draw_fruit_shadow();
	void jump(float3 direction, float power);
	void setStartPosition(float3 pos);
	void setNextDestination(float3 nextDest);
	void setWorldHome(float3 pos);
	void bindToEnvironment(Environment* terrain);
	bool withinDistanceTo(float3 target, float treshhold);
	ParticleSystem* getParticleSystem();
	float3 getHomePosition() const;

	Skillshot hit(float3 playerPos);

	FruitType getFruitType();

	static shared_ptr<Fruit> createFruitFromType(FruitType type);
};
