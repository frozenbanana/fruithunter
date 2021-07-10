#pragma once
#include "Fruit.h"
class Pomegranate : public Fruit {
private:

	bool m_jumping = false;
	float m_charge = 0;
	float m_jumpAnim = 0;

	void behavior(float dt);

public:

	void update();

	void updateAnimated(float dt);

	Pomegranate(float3 position = float3(0.f));
};
