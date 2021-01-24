#include "Menu_PoppingButton.h"
#include "AudioController.h"

void Menu_PoppingButton::setDesiredScale_hovering(float value) { m_scale_desired_hovering = value; }

void Menu_PoppingButton::setDesiredScale_standard(float value) { m_scale_desired_standard = value; }

bool Menu_PoppingButton::update_behavior(float dt) {
	if (m_timer <= m_delay && m_timer + dt > m_delay) {
		SoundID id = AudioController::getInstance()->play("bubble_pop");
	}
	m_timer += dt;
	bool clicked = false;
	if (m_timer > m_delay) {
		setRotation(cos(m_timer - m_delay * 5) * 0.1f);
		clicked = update(dt);
		if (isHovering()) {
			m_scale_desired = m_scale_desired_hovering;
		}
		else {
			m_scale_desired = m_scale_desired_standard;
		}
		m_scale_velocity += (m_scale_desired - getScale().x) * m_scale_spring_speed * dt; // add force
		m_scale_velocity *= pow(m_scale_spring_friction, dt);							// friction
		setScale(getScale().x + m_scale_velocity * dt); // move scale as spring
	}
	return clicked;
}

void Menu_PoppingButton::set(float2 position, string text, float _delay) {
	m_delay = _delay;
	m_timer = 0;
	m_scale_velocity = 0;
	setScale(0);
	setPosition(position);

	static const vector<string> btns = { "btn_v1.png", "btn_v2.png", "btn_v3.png" };
	load(btns[rand() % btns.size()]);
	setText(text);
}

Menu_PoppingButton::Menu_PoppingButton() {}
