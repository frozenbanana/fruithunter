#pragma once
#include "Entity.h"

class Bow {
public:
	Bow();
	~Bow();
	void update(float dt, float3 playerPos, float3 playerForward, float3 playerRight);
	void draw();
	void rotate(float pitch, float yaw);
	void aim();
	void release();
	void charge();
	void shoot(float3 direction);

private:
	Entity m_bow;
	Entity m_arrow;

	float3 m_arrowDirection;



	bool m_ready = true;
	bool m_aiming = false;
	bool m_charging = false;
	bool m_shooting = false;
	bool m_chargeReset = true;

	float m_armLength = 1.0f;
	float m_arrowSpeed = 10.0f;
	float m_drawFactor = 0.0f;
	float m_aimMovement = 0.0f;
};