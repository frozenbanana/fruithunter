#include "Menu_PoppingButton.h"
#include "AudioController.h"

void Menu_PoppingButton::setDesiredScale_hovering(float value) { m_scale_desired_hovering = value; }

void Menu_PoppingButton::setDesiredScale_standard(float value) { m_scale_desired_standard = value; }

bool Menu_PoppingButton::update_behavior(float dt) {
	Input* ip = Input::getInstance();
	float2 mp = ip->mouseXY();
	bool clicked = false;
	if (m_timer <= m_delay && m_timer + dt > m_delay) {
		SoundID id = AudioController::getInstance()->play("bubble_pop");
		AudioController::getInstance()->setPitch(id, RandomFloat(-1, 1) * 0.5);
	}
	m_timer += dt;
	if (m_timer > m_delay) {
		setRotation(cos(m_timer - m_delay * 5) * 0.1f);
		clicked = update(dt, mp);
		if (isHovering(mp)) {
			m_scale_desired = m_scale_desired_hovering;
		}
		else {
			m_scale_desired = m_scale_desired_standard;
		}
		m_scale_velocity +=
			(m_scale_desired - getScale().x) * m_scale_spring_speed * dt; // add force
		m_scale_velocity *= pow(m_scale_spring_friction, dt);			  // friction
		setScale(getScale().x + m_scale_velocity * dt);					  // move scale as spring
	}
	return clicked;
}

void Menu_PoppingButton::set(float2 position, string text, float _delay) {
	m_delay = _delay;
	m_timer = 0;
	m_scale_velocity = 0;
	setScale(0);
	setTextScale(1);
	setPosition(position);

	static const vector<string> btns = { "btn_v1.png", "btn_v2.png", "btn_v3.png" };
	load(btns[rand() % btns.size()]);
	setText(text);
	setFont("myfile.spritefont");

	// standard colors
	setStandardColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
	setHoveringColor(Color(1.f, 210.f / 255.f, 0.f));
	setTextStandardColor(Color(1.f, 1.f, 1.f));
	setTextHoveringColor(Color(0.f, 0.f, 0.f));
}

Menu_PoppingButton::Menu_PoppingButton() {}
