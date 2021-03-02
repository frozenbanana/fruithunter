#include "Slider.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"
#include "Input.h"

float2 Slider::getSliderPos() const { 
	return m_position + m_sliderOffset + float2((m_value * 240.f) - 120.f, 0);
}

Slider::Slider() {
	m_img_background.load("sliderBackground.png");
	m_img_background.setScale(1.f);
	m_img_background.setAlignment(); // center

	m_img_grabber.load("apple.png");
	m_img_grabber.setScale(m_scale);
	m_img_grabber.setAlignment(); // center
}

Slider::~Slider() {}

void Slider::initialize(string label, float2 pos) {
	m_label = label;
	m_position = pos;
}

float Slider::getValue() { return m_value; }

void Slider::setPosition(float2 position) {
	float offset = position.x - m_position.x;
	m_position = position;
}

void Slider::setValue(float value) {
	m_value = value;
}

bool Slider::update() {
	Input* ip = Input::getInstance();
	bool changed = false;

	float2 mp = ip->mouseXY();
	float2 pos = m_img_grabber.getPosition();
	float radius = m_img_grabber.getSize().x * 0.5;

	m_colour = (m_sliding || (mp - pos).Length() < radius) ? m_grabberColor_highlighted
														   : m_grabberColor_standard;
	if ((mp-pos).Length() < radius) {
		// hovering
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			// clicked
			m_grabPos = mp.x;
			m_sliding = true;
			m_preValue = m_value;
		}
	}
	if (m_sliding && ip->mouseDown(Input::MouseButton::LEFT)) {
		// sliding btn
		float offset = mp.x - m_grabPos;
		m_value = Clamp<float>(m_preValue + offset / 240, 0, 1);
	}
	if (m_sliding && ip->mouseReleased(Input::MouseButton::LEFT)) {
		// release btn
		changed = true;
		m_sliding = false;
	}

	return changed;
}

void Slider::draw() {
	float2 sliderValuePos = getSliderPos();
	m_img_background.setPosition(m_position + float2(150.f, 0.f));
	m_img_background.draw();
	m_img_grabber.setPosition(sliderValuePos);
	m_img_grabber.setColor(m_colour);
	m_img_grabber.draw();

	m_textRenderer.setScale(m_scale * 8.0f);
	m_textRenderer.setAlignment(HorizontalAlignment::AlignRight, VerticalAlignment::AlignCenter);
	m_textRenderer.setPosition(m_position);
	m_textRenderer.setText(m_label + ":");
	m_textRenderer.draw();

	m_textRenderer.setScale(m_scale * 2.5f);
	m_textRenderer.setAlignment(); // center
	m_textRenderer.setPosition(sliderValuePos);
	m_textRenderer.setText(to_string(int(100 * m_value)) + "%");
	m_textRenderer.draw();
}
