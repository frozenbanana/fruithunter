#pragma once
#include "Fruit.h"
class Banana : public Fruit {
private:
	enum State { Jump, Bounce, Stopped };
	float m_maxBounciness;
	float m_bounciness;
	State m_state;

	float m_velocitySpeed_max = 10;

	float3 m_rotation;

	void playSound_bounce();
	void bounce(float3 normal, float bounceReflectMultiplier = 1.f, float randomHorizontalMultiplier = 0.f);

	void updateFirstJump(float dt);
	void updateBounce(float dt);
	void updateStopped(float dt);
	void bounce();
	void rotRandom() {
		m_rotation =
			float3((float)(rand() % 100), (float)(rand() % 10), (float)(rand() % 100)) * 0.1f;
	};

	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;

	float3 m_bounceDestination;

public:
	Banana(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void release(float3 direction) override;
};
