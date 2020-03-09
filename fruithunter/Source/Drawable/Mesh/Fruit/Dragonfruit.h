#pragma once
#include "AI.h"
#include "Fruit.h"

#define MAXNROFJUMPS 2
class Dragonfruit : public Fruit {
private:
	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;

	float m_offsetFromGround;
	float m_wingStrength;
	float m_passiveTurnSpeed;
	float m_activeTurnSpeed;
	float m_flapWings; // Y value when dragonfruit should flap wings to gain altitude.


	bool inFlight;

public:
	Dragonfruit(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void waveFlight(float3 playerDir, float radius);
	void circulateVertical(float3 playerDir, float radius);
};
