#pragma once
#include "AI.h"
#include "Fruit.h"

class Apple : public Fruit {
private:
	void behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) override;
	void behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) override;
	void behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) override;

public:
	Apple(float3 pos = float3(12.f, 0.f, 6.f));
	void updateAnimated(float dt);
	void flee(float3 playerDir, vector<shared_ptr<Entity>> collidables);
};
