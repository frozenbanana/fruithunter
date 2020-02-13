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
	float3 m_arrowForward;

	bool m_ready = true;
	bool m_aiming = false;
	bool m_charging = false;
	bool m_shooting = false;
	bool m_chargeReset = true;

	float m_armLength = 1.0f;
	float m_aimMovement = 0.0f;

	float m_drawFactor = 0.0f;
	float m_bowMass = 1.4f;

	//float m_dragNumber = 1.0f; //Assuming Fluid density * dragCoefficient * 0.5 = 1.
	float m_arrowArea = 0.0001;
	float m_arrowMass = 0.1f;
	float m_arrowLength = 0.5f;
	float m_arrowRotation = 0.0f;
	float3 m_oldArrowDirection = float3(0.f, 0.f, 0.f);

	void arrowPhysics(float dt, float3 windVector);
	void calcArea(float3 relativeWindVector);
};