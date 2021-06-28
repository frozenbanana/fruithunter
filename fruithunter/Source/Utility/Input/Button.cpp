#include "Button.h"
#include "Input.h"
#include "ErrorLogger.h"
#include "Renderer.h"

Button::Button() {
	m_text.setFont("lato.spritefont");
	m_text.setScale(0.4f);
	m_text.setAlignment();
}

Button::~Button() {}

void Button::setLabel(string label) { m_label = label; }

void Button::initialize(string label, float2 position) {
	m_label = label;
	setPosition(position);
	m_colour = COL_INACTIVE;
	m_text.setText(m_label);
}

void Button::initialize(string label, float2 position, bool on) {
	m_label = label;
	setPosition(position);
	m_on = on;
	m_isToggle = true;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": On");
}

void Button::initialize(string label, float2 position, Setting value) {
	m_label = label;
	setPosition(position);
	m_isLowMedHighUltra = true;
	m_lowMedHighUltra = value;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": Ultra");
}

void Button::initialize(string label, float2 position, Resolution value) {
	m_label = label;
	setPosition(position);
	m_isResolution = true;
	m_resolution = value;
	m_colour = COL_INACTIVE;
	m_text.setText(m_label + ": 3840x2160");
}

void Button::setLowMedHighUltra(Setting value) { m_lowMedHighUltra = value; }

void Button::setResolution(Resolution value) { m_resolution = value; }

void Button::setOnOff(bool value) { m_on = value; }

BoundingBox2D Button::getBoundingBox() const {
	BoundingBox2D bb;
	bb.set(Matrix::CreateTranslation(getPosition().x, getPosition().y, 0), m_text.getSize(),
		float2(0, 0));
	return bb;
}

bool Button::getOnOff() { return m_on; }

int Button::getLowMedHighUltra() { return m_lowMedHighUltra; }

int Button::getResolution() { return m_resolution; }

bool Button::update() {
	Input* ip = Input::getInstance();
	bool clicked = false;

	if (getBoundingBox().isInside(ip->mouseXY())) {
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
	m_text.setPosition(getPosition());
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
