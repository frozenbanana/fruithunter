#pragma once
#include "Fruit.h"


class Melon : public Fruit {
private:
	float m_rollAnimationSpeed;
	float3 m_secondWorldHome;
	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;

	void roll(float dt);
	float m_rollSpeed;
	int m_angleDirection;
	int m_triesToGoHome;
public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void setRollSpeed(float rollSpeed);
	float3 circulateAround(float3 playerPosition);
	void pathfinding(float3 start, std::vector<float4>* animals) override;
};
