#pragma once
#include "Fruit.h"
class Banana : public Fruit {
private:
	enum State { Jump, Bounce, Stopped };
	float m_maxBounciness;
	float m_bounciness;
	State m_state;

	float3 m_rotation;

	void updateFirstJump(float dt);
	void updateBounce(float dt);
	void updateStopped(float dt);
	void stop();
	void bounce();
	void rotRandom() { m_rotation = float3(rand() % 100, rand() % 10, rand() % 100) * 0.1; };

public:
	Banana(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
};
