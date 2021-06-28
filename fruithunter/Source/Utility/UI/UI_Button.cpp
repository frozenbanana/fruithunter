#include "UI_Button.h"
#include "Renderer.h"

void UI_Button::_draw(const Transformation2D& transformation) {
	// background image
	m_sprite.draw(transformation);

	// text
	m_text.setScale(0.5f * m_textScale);
	m_text.draw(transformation);
}

float2 UI_Button::getLocalSize() const { return m_sprite.getLocalSize(); }

void UI_Button::setText(string text) { m_text.setText(text); }

void UI_Button::setFont(string font) { m_text.setFont(font); }

void UI_Button::setColor(Color color) { m_sprite.setColor(color); }

void UI_Button::setAlpha(float alpha) {
	m_sprite.setAlpha(alpha);
	m_text.setAlpha(alpha);
}

void UI_Button::setTextScale(float scale) { m_textScale = scale; }

void UI_Button::setStandardColor(Color color) { m_color_standard = color; }

void UI_Button::setHoveringColor(Color color) { m_color_hovering = color; }

void UI_Button::setTextStandardColor(Color color) { m_textColor_standard = color; }

void UI_Button::setTextHoveringColor(Color color) { m_textColor_hovering = color; }

void UI_Button::setColorChangeTime(float time) { m_colorChangeTime = time; }

bool UI_Button::update(float dt, float2 mp) {
	bool clicked = false;
	if (getBoundingBox().isInside(mp)) {
		// mouse hovering
		m_interpolation = Clamp(m_interpolation + (dt * (1.f / m_colorChangeTime)), 0.f, 1.f);
		if (Input::getInstance()->mousePressed(m_key_activator))
			clicked = true;
	}
	else {
		m_interpolation = Clamp(m_interpolation - (dt * (1.f / m_colorChangeTime)), 0.f, 1.f);
	}
	setColor((m_color_standard * (1 - m_interpolation)) + (m_color_hovering * m_interpolation));
	m_text.setColor(
		(m_textColor_standard * (1 - m_interpolation)) + (m_textColor_hovering * m_interpolation));
	return clicked;
}

void UI_Button::load(string image) {
	if (image != "")
		m_sprite.load(image);
}

void UI_Button::_imgui_properties() {
	ImGui::ColorEdit4("Color Standard", (float*)&m_color_standard);
	ImGui::ColorEdit4("Color Hover", (float*)&m_color_hovering);
	ImGui::ColorEdit4("Text Color Standard", (float*)&m_textColor_standard);
	ImGui::ColorEdit4("Text Color Standard", (float*)&m_textColor_hovering);
}

UI_Button::UI_Button(string image) {
	m_text.setAlignment();	 // middle center
	m_sprite.setAlignment(); // middle center
	load(image);
}
