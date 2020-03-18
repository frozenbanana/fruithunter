#pragma once
#include "Fruit.h"

#define MAXNROFJUMPS 2
class Apple : public Fruit {
private:
	void behaviorPassive(float3 playerPosition) override;
	void behaviorActive(float3 playerPosition) override;
	void behaviorCaught(float3 playerPosition) override;
	int m_nrOfJumps = 0;

public:
	Apple(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void flee(float3 playerDir);
	void pathfinding(float3 start, std::vector<float4>* animals) override;
};
