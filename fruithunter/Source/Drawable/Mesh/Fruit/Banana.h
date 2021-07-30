#pragma once
#include "Fruit.h"

class Banana : public Fruit {
private:
	void playSound_bounce();

	bool validAndSecureJumpTarget(float3 source, float3 target, float maximumJumpHeight);

	void _onDeath(Skillshot skillshot) override;

	void behavior() override;

public:
	Banana(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);

	void update() override;

};
