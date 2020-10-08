#pragma once
#include "Fruit.h"
#include "Timer.h"
#include "AI.h"
#include "Fruit.h"

class DragonFruit : public Fruit {
private:
	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;
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

	float m_heightFromGroundLimit = 20; // If under limit, will force fruit to fly away from ground
	float m_velocitySpeed = 10; // flight speed
	float m_speedGain = 8; // speed acquired from tilting
	float m_steeringSpeed = 1.2; // speed of turning to target
	float m_turningStrength = 50; // z rotation strength from turning
	float m_rotZ = 0; // current z rotation

	float m_timer = 0; // time until new target 
	float2 m_timer_range = float2(3,5); // timer length range

public:
	DragonFruit(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void pathfinding(float3 start, std::vector<float4>* animals);
	float3 getRandomTarget();
};
