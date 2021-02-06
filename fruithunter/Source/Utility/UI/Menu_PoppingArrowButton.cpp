#include "Menu_PoppingArrowButton.h"
#include "AudioController.h"

void Menu_PoppingArrowButton::setDesiredScale_hovering(float value) {
	m_scale_desired_hovering = value;
}

void Menu_PoppingArrowButton::setDesiredScale_standard(float value) {
	m_scale_desired_standard = value;
}

bool Menu_PoppingArrowButton::update_behavior(float dt) { 
	if (m_timer <= m_delay && m_timer + dt > m_delay) {
		SoundID id = AudioController::getInstance()->play("bubble_pop");
		AudioController::getInstance()->setPitch(id, RandomFloat(-1, 1) * 0.5);
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
		if (clicked) {
			m_scale_velocity = m_scale_desired * 3;
			SoundID id = AudioController::getInstance()->play("bubble_pop");
			AudioController::getInstance()->setPitch(id, RandomFloat(-1, 1) * 0.5);
		}
		m_scale_velocity +=
			(m_scale_desired - getScale().x) * m_scale_spring_speed * dt; // add force
		m_scale_velocity *= pow(m_scale_spring_friction, dt);			  // friction
		setScale(getScale().x + m_scale_velocity * dt);// move scale as spring
		setPosition(m_startPosition + float2(1, 0) * 10 * (m_direction*2-1) * cos(m_timer*1.82f - m_delay*5));
	}
	return clicked;
}

void Menu_PoppingArrowButton::set(
	float2 position, string text, float _delay, Menu_PoppingArrowButton::FacingDirection direction) {
	m_delay = _delay;
	m_direction = direction;
	m_timer = 0;
	m_scale_velocity = 0;
	setScale(0);
	m_startPosition = position;
	setPosition(m_startPosition);

	load(direction ? "btn_arrowRight.png":"btn_arrowLeft.png");
}

Menu_PoppingArrowButton::Menu_PoppingArrowButton() {}
