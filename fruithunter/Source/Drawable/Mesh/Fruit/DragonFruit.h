#pragma once
#include "Fruit.h"

class DragonFruit : public Fruit {
private:
	float3 m_target;
	float m_offsetFromGround;
	float m_wingStrength;
	float m_passiveTurnSpeed;
	float m_activeTurnSpeed;
	bool isFalling();
	bool m_ascend = false;
	bool updated = false;

	float m_animationSpeed_base = 4;
	float m_animationSpeed_changeOnTilt = 4;
	float m_animationSpeed = m_animationSpeed_base;

	float2 m_spawn_range = float2(5,35); // additional height above terrain height

	float m_heightFromGroundLimit =
		20.0f;							// If under limit, will force fruit to fly away from ground
	float m_velocitySpeed = 10.0f;		// flight speed
	float m_speedGain = 8.0f;			// speed acquired from tilting
	float m_steeringSpeed = 1.2f; // speed of turning to target
	float m_turningStrength = 50.0f; // z rotation strength from turning
	float m_rotZ = 0.0f;			 // current z rotation

	float m_timer = 0.0f;				 // time until new target 
	float2 m_timer_range = float2(3,5); // timer length range

	void _onDeath(Skillshot skillshot) override;

	void behavior() override;

public:
	DragonFruit(float3 pos);
	void updateAnimated(float dt);
	void pathfinding(float3 start, std::vector<float4>* animals);
	float3 getRandomTarget();

	void update() override;

};
