#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "Text2D.h"
#include "Input.h"

class UI_Button : public Drawable2D {
private:
	Sprite2D m_sprite;
	Text2D m_text;

	Color m_color_standard = Color(1, 1, 1, 1);
	Color m_color_hovering = Color(1, 1, 1, 1);
	Color m_textColor_standard = Color(0, 0, 0, 1);
	Color m_textColor_hovering = Color(0, 0, 0, 1);

	float m_colorChangeTime = 0.1f;
	float m_interpolation = 0.f;
	float m_textScale = 1.f;

	Input::MouseButton m_key_activator = Input::MouseButton::LEFT;

	// -- Private Functions --
	void _draw(const Transformation2D& transformation);
	void _imgui_properties();

public:
	float2 getLocalSize() const;

	void setText(string text);
	void setFont(string font);
	void setColor(Color color);
	void setAlpha(float alpha);
	void setTextScale(float scale);

	void setStandardColor(Color color);
	void setHoveringColor(Color color);

	void setTextStandardColor(Color color);
	void setTextHoveringColor(Color color);

	void setColorChangeTime(float time);

	/* Returns true if clicked */
	bool update(float dt, float2 mp);

	void load(string image);

	UI_Button(string image = "");

};
