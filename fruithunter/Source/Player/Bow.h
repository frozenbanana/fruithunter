#pragma once
#include "Entity.h"

class Bow {
public:
	Bow();
	~Bow();
	void update(float dt, float3 playerPos, float3 playerForward, float3 playerRigh);
	void draw();
	void rotate(float pitch, float yaw);
	void aim();
	void release();
	void charge();
	void shoot(float3 direction);

	Entity& getArrow() { return m_arrow; };

private:
	Entity m_bow;
	Entity m_arrow;

	float3 m_arrowVelocity;

	bool m_ready = true;
	bool m_aiming = false;
	bool m_charging = false;
	bool m_shooting = false;
	bool m_chargeReset = true;

	float m_armLength = 0.75f;
	float m_aimMovement = 0.0f;

	float m_drawFactor = 0.0f;
	float m_bowMass = 1.4f;

	// Assuming Fluid density * dragCoefficient * 0.5 = 1.
	float m_arrowArea = 0.0001f;
	float m_arrowMass = 0.1f;
	float m_arrowLength = 0.5f;
	float m_arrowRotation = 0.0f;
	float3 m_oldArrowVelocity = float3(0.f, 0.f, 0.f);

	void arrowPhysics(float dt, float3 windVector);
	void calcArea(float3 relativeWindVector);
	float calcAngle(float3 vec1, float3 vec2);
};