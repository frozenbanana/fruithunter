#pragma once
#include "Translation2DStructures.h"
#include "Sprite2D.h"
#include "TextRenderer.h"
#include "Input.h"

class UI_Button : public Transformation2D {
private:
	Sprite2D m_sprite;
	TextRenderer m_textRenderer;
	string m_text;

	Color m_color_standard = Color(1, 1, 1, 1);
	Color m_color_hovering = Color(1, 1, 1, 1);
	Color m_textColor_standard = Color(0, 0, 0, 1);
	Color m_textColor_hovering = Color(0, 0, 0, 1);

	Input::MouseButton m_key_activator = Input::MouseButton::LEFT;

	// -- Private Functions --
	bool mouseOn(float2 mp) const;

public:
	bool isHovering() const;

	void setText(string text);
	void setFont(string font);
	void setColor(Color color);

	void setStandardColor(Color color);
	void setHoveringColor(Color color);

	void setTextStandardColor(Color color);
	void setTextHoveringColor(Color color);

	/* Returns true if clicked */
	bool update();

	void draw();

	void load(vector<string> images, float animationSpeed = 1.f);
	void load(string image);

	UI_Button(vector<string> images = vector<string>(), float animationSpeed = 1.f);

};
