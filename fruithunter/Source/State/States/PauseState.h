#pragma once
#include "StateItem.h"
#include "Entity.h"
#include "Sprite2D.h"
#include "Menu_PoppingButton.h"
#include "Timer.h"

class PauseState : public StateItem {
private:
	Timer m_timer;
	Sprite2D m_settingsBackground;

	enum MenuButtons { btn_restart, btn_resume, btn_controls, btn_settings, btn_mainmenu, btn_exit, btn_length };
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
