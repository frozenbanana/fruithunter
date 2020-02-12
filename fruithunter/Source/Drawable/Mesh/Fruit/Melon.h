#pragma once
#include "Fruit.h"

class Melon : public Fruit {
private:
public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void update(float dt, float3 playerPosition, Terrain* terrain);
	void move(float dt);
	void circulateAround(float3 playerPosition);
};
