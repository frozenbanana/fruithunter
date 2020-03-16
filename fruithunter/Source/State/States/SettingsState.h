#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Slider.h"

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

	Button m_vsyncButton;
	Button m_darkEdgesButton;
	Button m_backButton;
	Button m_shadowsButton;
	Button m_resolutionButton;
	Button m_fullscreenButton;

	Slider m_drawDistance;
	Slider m_masterVolume;
	Slider m_musicVolume;
	Slider m_effectsVolume;

	Entity m_entity;
};
