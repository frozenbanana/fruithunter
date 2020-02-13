#pragma once
#include "Fruit.h"

class Melon : public Fruit {
private:
	// struct Phase {
	//	float3 position;
	//	float phaseSpeed;
	//	float3 rotationSpeed;

	//	Phase(float3 pos, float fspeed, float3 rotSpeed) {
	//		position = pos;
	//		phaseSpeed = fspeed;
	//		rotationSpeed = rotSpeed;
	//	}
	//};

	float m_rollSpeed;

	// void updatePhases(Phase phases[]);
	void roll(float dt);

public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void setRollSpeed(float rollSpeed);
};
