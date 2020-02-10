#pragma once
#include "Fruit.h"
class Apple : public Fruit {
private:
public:
	Apple(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
};
