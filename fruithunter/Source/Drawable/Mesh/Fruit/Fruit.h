#pragma once
#include "Entity.h"
#include "AI.h"
#include "TerrainManager.h"
#include "ParticleSystem.h"

#define THROWVELOCITY 30.f
#define LONGSHOT 17.f
#define MEDIUMSHOT 10.f
#define FASTMOVING_VELOCITY 11.f

class Fruit : public Entity, public AI {
protected:
	// Phyics based movment
	float3 m_velocity = float3(0.f);
	float m_speed = 0.0f;
	float3 m_gravity = float3(0.0f, -1.0f, 0.0f) * 15.0f; // same as player
	float3 m_direction;
	float m_groundFriction = 10.f;
	float m_airFriction = 20.f;

	bool m_onGround = true;

	void checkOnGroundStatus();

	float m_passive_speed;
	float m_active_speed;
	float m_caught_speed;
	unique_ptr<ParticleSystem> m_particleSystem;
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
	size_t m_nrOfTriesGoHome = 0;
	int m_fruitType;
	float m_startRotation; // start and end to interpolate between.
	float m_endRotation;
	void setAnimationDestination();
	void enforceOverTerrain();
	Fruit(float3 pos = float3(0.f, 0.f, 0.f));
	void behaviorReleased() override;
	void behaviorInactive(float3 playerPosition) override;

	void setDirection();


	bool m_afterRealease = false;

	void updateVelocity(float dt);
	void stopMovement();

public:
	virtual void release(float3 direction);
	void move(float dt);
	void update(float dt, float3 playerPosition);
	virtual void updateAnimated(float dt) = 0;
	void jump(float3 direction, float power);
	void setStartPosition(float3 pos);
	void setNextDestination(float3 nextDest);
	void setWorldHome(float3 pos);
	bool withinDistanceTo(float3 target, float treshhold);
	ParticleSystem* getParticleSystem();
	float3 getHomePosition() const;


	void hit(float3 playerPos) {
		changeState(CAUGHT);
		float dist = (playerPos - m_position).Length();
		float4 colors[3];
		int nrOf = 5;
		if (dist > LONGSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// gold
				colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
				colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
				colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
				nrOf = 22;
			}
			else {
				// gold
				colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
				colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
				colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
				nrOf = 12;
			}
		}
		else if (dist > MEDIUMSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// case 2: Medium shot
				// in air or fast moving -> gold
				// Gold
				colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
				colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
				colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
				nrOf = 8;
			}
			else {
				// silver
				colors[0] = float4(0.75f, 0.75f, 0.75f, 1.0f);
				colors[1] = float4(0.75f, 0.75f, 0.75f, 1.0f);
				colors[2] = float4(0.75f, 0.75f, 0.75f, 1.0f);
				nrOf = 13;
			}
		}
		else {
			// bronze
			colors[0] = float4(0.69f, 0.34f, 0.05f, 1.0f);
			colors[1] = float4(0.71f, 0.36f, 0.07f, 1.0f);
			colors[2] = float4(0.70f, 0.32f, 0.09f, 1.0f);
			nrOf = 6;
		}
		m_particleSystem->setColors(colors);
		m_particleSystem->setAmountOfParticles(nrOf);
		m_particleSystem->run(true);
		m_currentMaterial = 2;
	}

	int getFruitType();
};
