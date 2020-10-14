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

	Input::MouseButton m_key_activator = Input::MouseButton::LEFT;

	bool m_hovering = false;
	bool m_clicked = false;

	// -- Private Functions --
	bool mouseOn(float2 mp);

public:
	bool isHovering() const;
	bool isClicked() const;

	void setText(string text);
	void setFont(string font);

	/* Returns true if clicked */
	bool update();

	void draw();

	void load(vector<string> images, float animationSpeed = 1.f);
	void load(string image);

	UI_Button(vector<string> images = vector<string>(), float animationSpeed = 1.f);

};
