#pragma once
#include "Fruit.h"

class Melon : public Fruit {
private:
public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
};
