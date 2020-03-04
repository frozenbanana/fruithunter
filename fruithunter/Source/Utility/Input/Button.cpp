#include "Button.h"
#include "Input.h"
#include "ErrorLogger.h"

Button::Button() {}

Button::~Button() {}

void Button::initialize(string label, float2 position) {
	m_label = label;
	m_position = position;
	m_colour = float4(1.f);
	m_size = m_textRenderer.getSize(m_label);
}

bool Button::update() {
	Input* ip = Input::getInstance();
	bool clicked = false;

	int x = abs(ip->mouseX() - m_position.x);
	int y = abs(ip->mouseY() - m_position.y);

	if (x < m_size.x / 2.f && y < m_size.y / 2.f) {
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			clicked = true;
		}
		m_colour = float4(1.f, 0.f, 0.f, 1.f);
	}
	else {
		m_colour = float4(1.f);
	}

	return clicked;
}

void Button::draw() { m_textRenderer.draw(m_label, m_position, m_colour); }
