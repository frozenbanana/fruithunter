#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"

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

	Entity m_entity;
};
