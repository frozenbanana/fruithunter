#include "Menu_Toggle.h"
#include "Input.h"

bool Menu_Toggle::getState() const { return m_state; }

bool Menu_Toggle::update(float dt, float2 mp) { 
	float2 lmp = float2::Transform(mp, getMatrix().Invert());
	if (m_toggle_left.update(dt, lmp))
		setState(true);
	if (m_toggle_right.update(dt, lmp))
		setState(false);
	return getState();
}

void Menu_Toggle::setState(bool state) { 
	m_state = state; 
	m_toggle_left.setState(m_state);
	m_toggle_right.setState(!m_state);
}

void Menu_Toggle::_draw(const Transformation2D& source) {
	m_text.setScale(0.5);
	m_text.draw(source);
	m_toggle_left.setPosition(float2(75, 0));
	m_toggle_left.draw(source);
	m_toggle_right.setPosition(float2(180, 0));
	m_toggle_right.draw(source);
}

bool ToggleSelectable::isHovering(float2 mp) const { 
	float2 localMP = float2::Transform(mp, getMatrix().Invert());
	return m_sprite.getBoundingBox().isInside(localMP);
}

bool ToggleSelectable::isClicked(float2 mp) const {
	Input* ip = Input::getInstance();
	return ip->mousePressed(Input::LEFT) && isHovering(mp);
}

void ToggleSelectable::_draw(const Transformation2D& source) {
	// background image
	m_sprite.draw(source);

	// text
	m_text.setScale(0.5);
	m_text.draw(source);
}

float2 ToggleSelectable::getSize() const { return m_sprite.getSize(); }

bool ToggleSelectable::getState() const { return m_state; }

void ToggleSelectable::setState(bool state) { m_state = state; }

bool ToggleSelectable::update(float dt, float2 mp) {
	bool state = getState();
	bool hovering = isHovering(mp);
	bool clicked = isClicked(mp);

	// state
	if (clicked)
		m_state = !m_state;

	// color interpolation
	int dir = (state ? 1 : -1);
	m_interpolation = Clamp(m_interpolation + dir*(dt * (1.f / m_colorChangeTime)), 0.f, 1.f);
	Color interpolatedColor =
		m_color_standard * (1 - m_interpolation) + m_color_hovering * m_interpolation;
	m_sprite.setColor(interpolatedColor);
	m_text.setColor(
		(m_textColor_standard * (1 - m_interpolation)) + (m_textColor_hovering * m_interpolation));

	// scale interpolation
	m_scale_desired = (hovering || state ? m_scale_desired_hovering : m_scale_desired_standard);
	m_scale_velocity += (m_scale_desired - getScale().x) * m_scale_spring_speed * dt; // add force
	m_scale_velocity *= pow(m_scale_spring_friction, dt); // friction
	m_scale_current = m_scale_current + m_scale_velocity * dt; // move scale as spring
	setScale(m_scale_current);

	return clicked;
}

void ToggleSelectable::init(string spriteFile, string text) {
	m_sprite.load(spriteFile);
	m_text.setText(text);
}

ToggleSelectable::ToggleSelectable() {
	m_text.setAlignment();
	m_sprite.setAlignment();
}

void Menu_Toggle::init(float2 position, string text) {
	setPosition(position);
	m_text.setText(text);

	m_toggle_left.init("btn_toggle_left.png", "ON");
	m_toggle_right.init("btn_toggle_right.png", "OFF");
}

Menu_Toggle::Menu_Toggle() { 
	setState(false);
	m_text.setAlignment(HorizontalAlignment::Right, VerticalAlignment::Center);
}