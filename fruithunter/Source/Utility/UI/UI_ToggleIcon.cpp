#include "UI_ToggleIcon.h"
#include "Input.h"

void UI_ToggleIcon::_draw(const Transformation2D& transformation) {
	m_background.draw(transformation);
	m_icon.draw(transformation);
}

void UI_ToggleIcon::updateState() {
	m_background.setColor(m_color_background[m_btnState]);
	m_icon.setColor(m_color_icon[m_btnState]);
}

float2 UI_ToggleIcon::getLocalSize() const { return m_background.getLocalSize(); }

void UI_ToggleIcon::setToggleState(bool state) {
	m_btnState = (state ? BtnState_Active : BtnState_Inactive);
}

bool UI_ToggleIcon::isActive() const { return m_btnState == BtnState_Active; }

void UI_ToggleIcon::setAlpha(float alpha) {
	m_background.setAlpha(alpha);
	m_icon.setAlpha(alpha);
}

bool UI_ToggleIcon::update(float dt, float2 mp) {
	bool clicked = false;
	if (!isActive()) {
		if (getBoundingBox().isInside(mp)) {
			// mouse hovering
			m_btnState = BtnState_Highlight;
			if (Input::getInstance()->mousePressed(Input::MouseButton::LEFT)) {
				clicked = true;
				m_btnState = BtnState_Active;
			}
		}
		else
			m_btnState = BtnState_Inactive;
	}
	updateState();
	return clicked;
}

UI_ToggleIcon::UI_ToggleIcon() {}
