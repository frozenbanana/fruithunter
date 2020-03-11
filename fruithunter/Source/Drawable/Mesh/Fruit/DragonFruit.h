#pragma once
#include "Fruit.h"

class DragonFruit : public Fruit {

public:
	DragonFruit();
	~DragonFruit();
	void updateAnimated(float dt);
};