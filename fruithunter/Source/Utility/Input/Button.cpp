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

void Button::initialize(string label, float2 position, Setting value) {
	m_label = label;
	m_position = position;
	m_isLowMedHighUltra = true;
	m_lowMedHighUltra = value;
	m_colour = float4(1.f);
	m_size = m_textRenderer.getSize(m_label + ": Ultra");
}

bool Button::getOnOff() { return m_on; }

int Button::getLowMedHighUltra() { return m_lowMedHighUltra; }

bool Button::update() {
	Input* ip = Input::getInstance();
	bool clicked = false;

	int x = abs(ip->mouseX() - (int)m_position.x);
	int y = abs(ip->mouseY() - (int)m_position.y);

	if (x < m_size.x / 2.f && y < m_size.y / 2.f) {
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			if (m_isToggle) {
				m_on = !m_on;
			}
			else if (m_isLowMedHighUltra) {
				m_lowMedHighUltra++;
				if (m_lowMedHighUltra > 3)
					m_lowMedHighUltra = 0;
			}

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
	else if (m_isLowMedHighUltra) {
		if (m_lowMedHighUltra == LOW)
			m_textRenderer.draw(m_label + ": Low", m_position, m_colour);
		else if (m_lowMedHighUltra == MEDIUM)
			m_textRenderer.draw(m_label + ": Medium", m_position, m_colour);
		else if (m_lowMedHighUltra == HIGH)
			m_textRenderer.draw(m_label + ": High", m_position, m_colour);
		else if (m_lowMedHighUltra == ULTRA)
			m_textRenderer.draw(m_label + ": Ultra", m_position, m_colour);
	}
	else
		m_textRenderer.draw(m_label, m_position, m_colour);
}
