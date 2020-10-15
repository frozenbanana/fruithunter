#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "Sprite2D.h"
#include "Menu_PoppingButton.h"
#include "Timer.h"

class PauseState : public StateItem {
private:
	Timer m_timer;
	TextRenderer m_textRenderer;
	Sprite2D m_settingsBackground;

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

	enum MenuButtons { btn_restart, btn_resume, btn_settings, btn_mainmenu, btn_exit, btn_length };
	Menu_PoppingButton m_buttons[btn_length];

public:
	PauseState();
	~PauseState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();

};
