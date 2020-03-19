#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Sprite2D.h"
#include "Timer.h"

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

	//tutorial sprites
	Sprite2D m_dropFruits[4];
	Sprite2D m_btns[5];
	Sprite2D m_movementKeys;
	Sprite2D m_charMovement;
	Sprite2D m_jumpKey;
	Sprite2D m_charJump;
	Sprite2D m_slowMo;
	Sprite2D m_bow;
	Sprite2D m_mouseLeft;

	int m_previousState;

	Entity m_entity;
};
