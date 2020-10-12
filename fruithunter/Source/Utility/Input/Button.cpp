#include "Button.h"
#include "Input.h"
#include "ErrorLogger.h"

Button::Button() {}

Button::~Button() {}

void Button::setLabel(string label) { m_label = label; }

void Button::setPosition(float2 position) { m_position = position; }

void Button::initialize(string label, float2 position) {
	m_label = label;
	m_position = position;
	m_colour = COL_INACTIVE;
	m_size = m_textRenderer.getSize(m_label);
}

void Button::initialize(string label, float2 position, bool on) {
	m_label = label;
	m_position = position;
	m_on = on;
	m_isToggle = true;
	m_colour = COL_INACTIVE;
	m_size = m_textRenderer.getSize(m_label + ": On");
}

void Button::initialize(string label, float2 position, Setting value) {
	m_label = label;
	m_position = position;
	m_isLowMedHighUltra = true;
	m_lowMedHighUltra = value;
	m_colour = COL_INACTIVE;
	m_size = m_textRenderer.getSize(m_label + ": Ultra");
}

void Button::initialize(string label, float2 position, Resolution value) {
	m_label = label;
	m_position = position;
	m_isResolution = true;
	m_resolution = value;
	m_colour = COL_INACTIVE;
	m_size = m_textRenderer.getSize(m_label + ": 3840x2160");
}

void Button::setLowMedHighUltra(Setting value) { m_lowMedHighUltra = value; }

void Button::setResolution(Resolution value) { m_resolution = value; }

void Button::setOnOff(bool value) { m_on = value; }

bool Button::getOnOff() { return m_on; }

int Button::getLowMedHighUltra() { return m_lowMedHighUltra; }

int Button::getResolution() { return m_resolution; }

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
				if (m_on) m_lowMedHighUltra++;
				if (m_lowMedHighUltra > 3)
					m_lowMedHighUltra = 0;
			}
			else if (m_isResolution) {
				if (m_on)
					m_resolution++;
				if (m_resolution > 3)
					m_resolution = 0;
			}

			clicked = true;
		}
		m_colour = COL_ACTIVE;
	}
	else {
		m_colour = COL_INACTIVE;
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
	else if (m_isResolution) {
		if (m_resolution == HD)
			m_textRenderer.draw(m_label + ": 1280x720", m_position, m_colour);
		else if (m_resolution == FHD)
			m_textRenderer.draw(m_label + ": 1920x1080", m_position, m_colour);
		else if (m_resolution == QHD)
			m_textRenderer.draw(m_label + ": 2560x1440", m_position, m_colour);
		else if (m_resolution == UHD)
			m_textRenderer.draw(m_label + ": 3840x2160", m_position, m_colour);
	}
	else
		m_textRenderer.draw(m_label, m_position, m_colour);
}
