#pragma once
#include "Drawable2D.h"
#include "Sprite2D.h"
#include "Text2D.h"
#include "Input.h"
#include "GlobalNamespaces.h"

class ToggleSelectable : public Drawable2D {
private:
	Sprite2D m_sprite;
	Text2D m_text;

	float m_scale_current = 1;
	float m_scale_desired = 0;
	float m_scale_desired_hovering = 0.9;
	float m_scale_desired_standard = 0.75;

	float m_scale_velocity = 0;
	const float m_scale_spring_speed = 200;
	const float m_scale_spring_friction = 0.000001;

	float m_colorChangeTime = 0.1f;
	float m_interpolation = 0.f;

	Color m_color_standard = Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f);
	Color m_color_hovering = Color(1.f, 210.f / 255.f, 0.f);
	Color m_textColor_standard = Color(1.f, 1.f, 1.f);
	Color m_textColor_hovering = Color(0, 0, 0);

	bool m_state = false;

	bool isHovering(float2 mp) const;
	bool isClicked(float2 mp) const;

	void _draw(const Transformation2D& source);

public:
	float2 getSize() const;

	bool getState() const;
	void setState(bool state);

	bool update(float dt, float2 mp);

	void init(string spriteFile = "", string text = "");

	ToggleSelectable();
};

class Menu_Toggle : public Drawable2D {
private:
	 
	ToggleSelectable m_toggle_left;
	ToggleSelectable m_toggle_right;
	Text2D m_text;

	bool m_state = false;
	
	void _draw(const Transformation2D& source);

public:
	bool getState() const;
	void setState(bool state);

	bool update(float dt, float2 mp);

	void init(float2 position, string text);

	Menu_Toggle();

};
