#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"

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
	Button m_exitButton;
	Button m_resumeButton;
	Button m_mainMenuButton;

	Entity m_entity;
};
