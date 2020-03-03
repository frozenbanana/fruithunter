#pragma once
#include "Fruit.h"
class Banana : public Fruit {
private:
	enum State { Jump, Bounce, Stopped };
	float m_maxBounciness;
	float m_bounciness;
	State m_state;

	float3 m_rotation;
	float3 m_acceleration;

	void updateFirstJump(float dt);
	void updateBounce(float dt);
	void updateStopped(float dt);
	void stop();
	void bounce();
	void rotRandom() {
		m_rotation =
			float3((float)(rand() % 100), (float)(rand() % 10), (float)(rand() % 100)) * 0.1f;
	};

	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;

	float3 m_bounceDestination;

public:
	Banana(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void release(float3 direction) override;
};
