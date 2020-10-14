#include "UI_Button.h"
#include "Renderer.h"

bool UI_Button::mouseOn(float2 mp) { 
	return m_sprite.getBoundingBox().isInside(mp);
}

bool UI_Button::isHovering() const { return m_hovering; }

bool UI_Button::isClicked() const { return m_clicked; }

void UI_Button::setText(string text) { m_text = text; }

void UI_Button::setFont(string font) { m_textRenderer.setFont(font); }

bool UI_Button::update() { 
	Input* ip = Input::getInstance();
	float2 mouse = float2(ip->mouseX(), ip->mouseY());
	if (mouseOn(mouse)) {
		// mouse hovering
		m_hovering = true;
		if (ip->mousePressed(m_key_activator)) {
			m_clicked = true;
		}
	}
	else {
		m_hovering = false;
		m_clicked = false;
	}
	return m_clicked;
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

