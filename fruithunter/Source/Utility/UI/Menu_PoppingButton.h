#pragma once
#include "UI_Button.h"
class Menu_PoppingButton : public UI_Button {
private:
	float m_delay = 0.f;
	float m_timer = 0.f;

	float m_scale_desired = 0.f;
	float m_scale_desired_hovering = 0.7f;//0.9
	float m_scale_desired_standard = 0.55f;//0.75

	float m_scale_velocity = 0.f;
	const float m_scale_spring_speed = 100.f;
	const float m_scale_spring_friction = 0.01f;

public:
	void setDesiredScale_hovering(float value);
	void setDesiredScale_standard(float value);

	bool update_behavior(float dt);
	void set(float2 position = float2(), string text = "", float _delay = 0);
	Menu_PoppingButton();
};
