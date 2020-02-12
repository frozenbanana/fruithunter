#pragma once
#include "AI.h"
#include "Fruit.h"

class Apple : public Fruit {
private:
	bool m_inAir = false;

public:
	Apple(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void move(float dt);
	void update(float dt, float3 playerPosition, float height);
	void flee(float3 playerDir);
};
