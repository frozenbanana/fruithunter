#pragma once
#include "Translation2DStructures.h"
#include "Sprite2D.h"

class UI_ToggleIcon : public Drawable2D {
protected:
	Sprite2D m_background, m_icon;

	enum ButtonState { BtnState_Inactive, BtnState_Highlight, BtnState_Active, BtnState_Length };
	ButtonState m_btnState = BtnState_Inactive;
	Color m_color_background[BtnState_Length] = { Color(1, 1, 1) * 0.5f, Color(1, 1, 1) * 1.f,
		Color(1, 1, 1) * 0.75f };
	Color m_color_icon[BtnState_Length] = { Color(1, 1, 1) * 0.5f, Color(1, 1, 1) * 1.f,
		Color(1, 1, 1) * 0.75f };

	// -- Private Functions --
	void _draw(const Transformation2D& transformation);

	void updateState();

public:
	float2 getLocalSize() const;

	void setToggleState(bool state);
	bool isActive() const;

	void setAlpha(float alpha);

	/* Returns true if clicked */
	bool update(float dt, float2 mp);

	UI_ToggleIcon();

};
