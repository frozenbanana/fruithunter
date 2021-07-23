#pragma once
#include "Fruit.h"

#define MAXNROFJUMPS 2
class Apple : public Fruit {
private:
	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;
	int m_nrOfJumps = 0;

	bool isValid(float3 point);

	void _onDeath(Skillshot skillshot) override;

public:
	Apple(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void flee(float3 playerDir);
	void pathfinding(float3 start) override;

	bool isRespawning() const;

};
