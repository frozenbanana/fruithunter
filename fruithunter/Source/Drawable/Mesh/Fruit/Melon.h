#pragma once
#include "Fruit.h"


class Melon : public Fruit {
private:
	float m_rollAnimationSpeed;
	float3 m_secondWorldHome;
	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;

	void roll(float dt);
	float m_rollSpeed;
	int m_angleDirection;
	int m_triesToGoHome;

	Entity m_ball;
	float3 m_sensors[8];
	bool m_sensorState[8];
	float3 m_sensorAvg;
	
	float m_topSpeed = 25;
	float m_acceleration = 50;
	float m_deacceleration = 30;
	float m_sensorHeightTrigger = 4;

public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void setRollSpeed(float rollSpeed);
	float3 circulateAround(float3 playerPosition);
	void pathfinding(float3 start, std::vector<float4>* animals) override;

	void draw_sensors();
};
