#include "Button.h"
#include "Input.h"
#include "ErrorLogger.h"



Button::Button() {}

Button::~Button() {}

void Button::setLabel(string label) { m_label = label; }

void Button::initialize(string label, float2 position) {
	m_label = label;
	m_position = position;
	m_colour = float4(1.f);
	m_size = m_textRenderer.getSize(m_label);
}

void Button::initialize(string label, float2 position, bool on) {
	m_label = label;
	m_position = position;
	m_on = on;
	m_isToggle = true;
	m_colour = float4(1.f);
	m_size = m_textRenderer.getSize(m_label + ": On");
}

bool Button::getOnOff() { return m_on; }

bool Button::update() {
	Input* ip = Input::getInstance();
	bool clicked = false;

	int x = abs(ip->mouseX() - (int)m_position.x);
	int y = abs(ip->mouseY() - (int)m_position.y);

	if (x < m_size.x / 2.f && y < m_size.y / 2.f) {
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			m_on = !m_on;
			clicked = true;
		}
		m_colour = float4(1.f, 0.f, 0.f, 1.f);
	}
	else {
		m_colour = float4(1.f);
	}

	return clicked;
}

void Button::draw() {
	if (m_isToggle && m_on)
		m_textRenderer.draw(m_label + ": On", m_position, m_colour);
	else if (m_isToggle && !m_on)
		m_textRenderer.draw(m_label + ": Off", m_position, m_colour);
	else
		m_textRenderer.draw(m_label, m_position, m_colour);
}
