#pragma once
#include "StateItem.h"
#include "Timer.h"
#include "Entity.h"
#include "Button.h"
#include "Slider.h"
#include "Sprite2D.h"

#include "Menu_PoppingButton.h"
#include "Menu_Toggle.h"

class SettingsState : public StateItem {
private:
	Timer m_timer;
	Sprite2D m_settingsBackground;
	Button m_vsyncButton;
	Button m_FXAAButton;
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

	Menu_Toggle m_toggle;

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
