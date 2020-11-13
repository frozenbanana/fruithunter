#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Slider.h"
#include "Sprite2D.h"

class SettingsState : public StateItem {
private:
	Sprite2D m_settingsBackground;
	Button m_vsyncButton;
	Button m_darkEdgesButton;
	Button m_shadowsButton;
	Button m_resolutionButton;
	Button m_fullscreenButton;

	Button m_applyButton;
	Button m_backButton;

	Slider m_drawDistance;
	Slider m_masterVolume;
	Slider m_musicVolume;
	Slider m_effectsVolume;
	Slider m_sensitivity;

	bool m_screenStateChanged = false;

public:
	SettingsState();
	~SettingsState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();

};
