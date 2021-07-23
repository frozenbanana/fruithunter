#pragma once
#include "Fruit.h"
class Pomegranate : public Fruit {
private:
	const float m_baseScale = 0.675f;

	bool m_jumping = false;
	float m_charge = 0;
	float m_jumpAnim = 0;

	bool m_ignited = false;
	float m_igniteTimer = 0;
	Skillshot m_hitSkillshot;


	void playSound_bounce();

	void behavior(float dt);

	bool validAndSecureJumpTarget(float3 source, float3 target, float maximumJumpHeight);

	void jumpToRandomLocation(float2 heightRange = float2(7.5f, 12.f), size_t samples = 16);

	void _onDeath(Skillshot skillshot) override;

public:

	void update();

	void updateAnimated(float dt);

	void onHit(Skillshot skillshot) override;

	Pomegranate(float3 position = float3(0.f));
};
