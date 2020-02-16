#pragma once
#include "AI.h"
#include "Fruit.h"

class Apple : public Fruit {
private:
	bool m_inAir = false;

	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;

public:
	Apple(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void move(float dt);
	void update(float dt, float3 playerPosition, TerrainManager* terrainManager);
	void flee(float3 playerDir);
};
