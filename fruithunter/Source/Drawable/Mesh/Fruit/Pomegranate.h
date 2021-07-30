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

	void _onDeath(Skillshot skillshot) override;

	void behavior() override;

public:

	void update() override;

	void updateAnimated(float dt) override;

	void onHit(Skillshot skillshot) override;

	Pomegranate(float3 position = float3(0.f));
};
