#pragma once
#include "Fruit.h"

#define MAXNROFJUMPS 2
class Apple : public Fruit {
private:
	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;
	int m_nrOfJumps = 0;
	float3 m_desiredLookDirection = float3(1, 0, 0);
	float3 m_currentLookDirection = m_desiredLookDirection;
	float rotationSpeed = 0.001f;

	bool isValid(float3 point);

	void jumpToLocation(float3 target);
	float3 findJumpLocation(float range, float maxHeight, size_t samples, float playerAvoidRange);

	bool validJumpTarget(float3 target);

	void playSound_bounce();

	void _onDeath(Skillshot skillshot) override;

public:
	Apple(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void flee(float3 playerDir);
	void pathfinding(float3 start) override;

	void restartAnimation();

	void update();

	bool isRespawning() const;
};
