#include "Menu_LeaderboardToggle.h"

void Menu_LeaderboardToggle::setIcon(string icon) { m_icon.load(icon); }

Menu_LeaderboardToggle::Menu_LeaderboardToggle(string icon) {
	setScale(0.5f);

	m_background.load("iconbtn.png");
	setIcon(icon);

	Color col_blue = Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f);
	Color col_yellow = Color(1.f, 204 / 255.f, 0, 1);

	m_color_background[ButtonState::BtnState_Inactive] = col_blue * 1.0f;
	m_color_background[ButtonState::BtnState_Highlight] = col_blue * 1.0f;
	m_color_background[ButtonState::BtnState_Active] = col_blue * 1.0f;

	m_color_icon[ButtonState::BtnState_Inactive] = col_blue * 0.5f;
	m_color_icon[ButtonState::BtnState_Highlight] = col_yellow * 0.65f;
	m_color_icon[ButtonState::BtnState_Active] = col_yellow;

	updateState();
}
