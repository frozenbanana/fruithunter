#pragma once
#include "Fruit.h"


class Melon : public Fruit {
private:
	void behaviorPassive() override;
	void behaviorActive() override;
	void behaviorCaught() override;

	ParticleSystem m_rollTrail;

	// Sensors
	Entity m_ball;
	float3 m_sensors[9];
	bool m_sensorState[9];
	float3 m_sensorAvg;
	bool m_showSensors = false;
	bool m_avoidPlayer = true;
	float m_sensorHeightTrigger = 4;
	float m_sensorWidthScale = 6.f;
	bool m_fixedSensors = true;

	// Movement params
	float m_topSpeed = 18;//20
	float m_acceleration = 0.15f;//0.05f
	float m_varyingForwardAngle = 1.5;
	float m_accumulatedTime = 0;
	float m_forwardAngleSpeed = 5;
	float m_gravityStrength = 25;

public:
	Melon(float3 pos = float3(0.f, 0.f, 0.f));
	void updateAnimated(float dt);
	void pathfinding(float3 start) override;

	void update();
	void update_imgui_changeParams();

	void draw_rollTrail();
	void draw_sensors();

	void draw_fruit();
};
