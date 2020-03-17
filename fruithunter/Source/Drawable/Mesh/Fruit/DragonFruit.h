#pragma once
#include "Fruit.h"

#include "AI.h"
#include "Fruit.h"

class DragonFruit : public Fruit {
private:
	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;

	float m_offsetFromGround;
	float m_wingStrength;
	float m_passiveTurnSpeed;
	float m_activeTurnSpeed;
	float m_flapWings; // Y value when dragonfruit should flap wings to gain altitude.

	bool isFalling();
	bool m_ascend = false;
	bool updated = false;
	void setDirection() override;
	void circulateVertical(float3 playerDir, float radius);
	void waveFlight(float3 playerDir);
public:
	DragonFruit(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void pathfinding(float3 start, std::vector<float4> *animals);
};
