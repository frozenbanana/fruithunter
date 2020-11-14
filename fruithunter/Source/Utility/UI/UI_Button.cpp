#include "UI_Button.h"
#include "Renderer.h"

bool UI_Button::mouseOn(float2 mp) const { 
	return m_sprite.getBoundingBox().isInside(mp);
}

bool UI_Button::isHovering() const { 
	Input* ip = Input::getInstance();
	return mouseOn(float2(ip->mouseX(), ip->mouseY()));
}

void UI_Button::setText(string text) { m_text = text; }

void UI_Button::setFont(string font) { m_textRenderer.setFont(font); }

void UI_Button::setColor(Color color) { m_sprite.setColor(color); }

void UI_Button::setStandardColor(Color color) { m_color_standard = color; }

void UI_Button::setHoveringColor(Color color) { m_color_hovering = color; }

void UI_Button::setTextStandardColor(Color color) { m_textColor_standard = color; }

void UI_Button::setTextHoveringColor(Color color) { m_textColor_hovering = color; }

void UI_Button::setColorChangeTime(float time) { m_colorChangeTime = time; }

bool UI_Button::update(float dt) { 
	bool clicked = false;
	if (isHovering()) {
		// mouse hovering
		m_interpolation = Clamp(m_interpolation + (dt * (1.f / m_colorChangeTime)), 0.f, 1.f);
		setColor((m_color_standard * (1-m_interpolation)) + (m_color_hovering * m_interpolation));
		m_textRenderer.setColor((m_textColor_standard * (1 - m_interpolation)) +(m_textColor_hovering * m_interpolation));
		if (Input::getInstance()->mousePressed(m_key_activator)) {
			clicked = true;
		}
	}
	else {
		m_interpolation = Clamp(m_interpolation - (dt * (1.f / m_colorChangeTime)), 0.f, 1.f);
		setColor((m_color_standard * (1 - m_interpolation)) + (m_color_hovering * m_interpolation));
		m_textRenderer.setColor((m_textColor_standard * (1 - m_interpolation)) + (m_textColor_hovering * m_interpolation));
	}
	return clicked;
}

void UI_Button::draw() { 
	float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));

	//background image
	m_sprite.setScale(getScale());
	m_sprite.setRotation(getRotation());
	m_sprite.setPosition(getPosition());
	m_sprite.draw();

	//text
	m_textRenderer.setScale(getScale().y * screenModifier.y);
	m_textRenderer.setRotation(getRotation());
	m_textRenderer.draw(m_text, getPosition() * screenModifier);
}

void UI_Button::load(vector<string> images, float animationSpeed) {
	m_sprite.load(images, animationSpeed);
}

void UI_Button::load(string image) {
	m_sprite.load(image);
}

UI_Button::UI_Button(vector<string> images, float animationSpeed) {
	m_textRenderer.setAlignment(); // middle center
	m_sprite.setAlignment(); // middle center
	load(images, animationSpeed);
}

