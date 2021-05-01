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

	float m_startScale = 0.5;
	float m_respawn_timer = 0;
	float m_respawn_max = 1;

public:
	Apple(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void flee(float3 playerDir);
	void pathfinding(float3 start) override;

	bool isRespawning() const;
};
