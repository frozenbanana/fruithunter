#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Slider.h"
#include "Sprite2D.h"

class SettingsState : public State {
public:
	SettingsState();
	~SettingsState();
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();

private:
	TextRenderer m_textRenderer;
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

	bool m_screenStateChanged = false;

	Entity m_entity;
};
