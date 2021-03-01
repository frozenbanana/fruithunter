#include "Button.h"
#include "Input.h"
#include "ErrorLogger.h"
#include "Renderer.h"

Button::Button() {}

Button::~Button() {}

void Button::setLabel(string label) { m_label = label; }

void Button::setPosition(float2 position) { m_position = position; }

void Button::initialize(string label, float2 position) {
	m_label = label;
	m_position = position;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label);
	m_size = m_text.getSize();
	m_text.setScale(m_size.y * 0.01f);
	m_text.setAlignment();
}

void Button::initialize(string label, float2 position, bool on) {
	m_label = label;
	m_position = position;
	m_on = on;
	m_isToggle = true;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": On");
	m_size = m_text.getSize();
	m_text.setScale(m_size.y * 0.01f);
	m_text.setAlignment();
}

void Button::initialize(string label, float2 position, Setting value) {
	m_label = label;
	m_position = position;
	m_isLowMedHighUltra = true;
	m_lowMedHighUltra = value;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": Ultra");
	m_size = m_text.getSize();
	m_text.setScale(m_size.y * 0.01f);
	m_text.setAlignment();
}

void Button::initialize(string label, float2 position, Resolution value) {
	m_label = label;
	m_position = position;
	m_isResolution = true;
	m_resolution = value;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": 3840x2160");
	m_size = m_text.getSize();
	m_text.setScale(m_size.y * 0.01f);
	m_text.setAlignment();
}

void Button::setLowMedHighUltra(Setting value) { m_lowMedHighUltra = value; }

void Button::setResolution(Resolution value) { m_resolution = value; }

void Button::setOnOff(bool value) { m_on = value; }

BoundingBox2D Button::getBoundingBox() const { 
	float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
	float2 position = m_position * screenModifier;
	float2 size = m_size * screenModifier;
	BoundingBox2D bb(position-size/2, position + size/2);
	return bb;
}

bool Button::getOnOff() { return m_on; }

int Button::getLowMedHighUltra() { return m_lowMedHighUltra; }

int Button::getResolution() { return m_resolution; }

bool Button::update() {
	Input* ip = Input::getInstance();
	float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
	bool clicked = false;

	float2 size = screenModifier * m_size;

	float2 mp = float2(ip->mouseX(), ip->mouseY());
	if (getBoundingBox().isInside(mp)) {
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			if (m_isToggle) {
				m_on = !m_on;
			}
			else if (m_isLowMedHighUltra) {
				m_lowMedHighUltra++;
				if (m_lowMedHighUltra > 3)
					m_lowMedHighUltra = 0;
			}
			else if (m_isResolution) {
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
	m_text.setColor(m_colour);
	m_text.setPosition(m_position);
	if (m_isToggle && m_on)
		m_text.setText(m_label + ": On");
	else if (m_isToggle && !m_on)
		m_text.setText(m_label + ": Off");
	else if (m_isLowMedHighUltra) {
		if (m_lowMedHighUltra == LOW)
			m_text.setText(m_label + ": Low");
		else if (m_lowMedHighUltra == MEDIUM)
			m_text.setText(m_label + ": Medium");
		else if (m_lowMedHighUltra == HIGH)
			m_text.setText(m_label + ": High");
		else if (m_lowMedHighUltra == ULTRA)
			m_text.setText(m_label + ": Ultra");
	}
	else if (m_isResolution) {
		if (m_resolution == HD)
			m_text.setText(m_label + ": 1280x720");
		else if (m_resolution == FHD)
			m_text.setText(m_label + ": 1920x1080");
		else if (m_resolution == QHD)
			m_text.setText(m_label + ": 2560x1440");
		else if (m_resolution == UHD)
			m_text.setText(m_label + ": 3840x2160");
	}
	else
		m_text.setText(m_label);
	m_text.draw();
}
