#pragma once
#include "Entity.h"
#include "ParticleSystem.h"
#include "TerrainBatch.h"

class Fruit : public Entity {
protected:
	FruitType m_fruitType;

	// Phyics based movement
	Environment* m_boundTerrain = nullptr;
	float3 m_velocity = float3(0.f);
	float3 m_gravity = float3(0.0f, -1.0f, 0.0f) * 15.0f; // same as player
	float m_groundFriction = 60.f;
	float m_airFriction = 60.f;
	bool m_onGround = true;

	// Respawning
	bool m_respawning = false;
	float m_startScale = 0.5;
	float m_respawn_timer = 0;
	float m_respawn_timeMax = 1;

	// Animation
	float m_frameTime = 0; // the value that is used to interpolate between key frames

	// Rendering
	bool m_isVisible = true;
	float3 m_baseColor = float3(1.f);

	// --- PRIVATE FUNCTIONS ---

	void enforceOverTerrain();

	void updateVelocity(float dt);

	void respawn();
	void updateRespawn();

	/* Intersects with Terrain and collidable Entitites. */
	bool rayCastWorld(float3 point, float3 forward, float3& intersection, float3& normal);
	/* Checks Terrain and Entity distance from feet and returns true if distance is less than specified threshold. */
	bool isOnGround(float3 position, float heightThreshold);

	void checkOnGroundStatus();

	void spawnCollectionPoint(Skillshot skillshot);

	void jumpToLocation(float3 target, float2 baseHeight = float2(1.f, 1.5f));
	float3 findJumpLocation(float range, float maxHeight, size_t samples, float playerAvoidRange);
	bool validJumpTarget(float3 target);

	virtual void _onDeath(Skillshot skillshot) = 0;

	virtual void behavior() = 0;


public:
	bool isVisible() const;
	void onDeath(Skillshot skillshot);
	void onHit();
	virtual void onHit(Skillshot skillshot);
	void move(float dt);
	virtual void updateAnimated(float dt) = 0;
	virtual void draw_fruit();
	virtual void draw_fruit_shadow();
	void bindToEnvironment(Environment* terrain);

	virtual void update() = 0;

	Skillshot hit(float3 playerPos);

	FruitType getFruitType();

	Fruit(FruitType type, float3 pos = float3(0.f, 0.f, 0.f));
};
