#pragma once
#include "Entity.h"

class Bow {
public:
	Bow();
	~Bow();
	void update(float dt, float3 playerPos, float3 playerForward);
	void draw(float3 playerRight);
	void rotate(float angleUp, float angleSide, float3 rotationAxis);
	void aim();
	void shoot(float3 direction);

private:
	Entity m_bow;
	Entity m_arrow;

	float3 m_arrowDirection;

	bool m_aiming = false;
	bool m_ready = true;
	bool m_shooting = false;
};