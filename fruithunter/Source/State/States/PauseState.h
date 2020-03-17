#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Sprite2D.h"

class PauseState : public State {
public:
	PauseState();
	~PauseState();
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();

private:
	TextRenderer m_textRenderer;
	Sprite2D m_settingsBackground;
	Button m_resumeButton;
	Button m_settingsButton;
	Button m_mainMenuButton;
	Button m_exitButton;
	Button m_restartButton;

	int m_previousState;

	Entity m_entity;
};
