#pragma once
#include "StateItem.h"
#include "Sprite2D.h"
#include "Menu_PoppingButton.h"
#include "Timer.h"
#include "Text2D.h"

class ControlState : public StateItem {
private:
	Timer m_timer;
	Text2D m_text;
	Sprite2D m_background;

	Sprite2D m_movementKeys;
	Sprite2D m_charMovement;
	Sprite2D m_jumpKey;
	Sprite2D m_charJump;
	Sprite2D m_slowMo;
	Sprite2D m_slowMoKey;
	Sprite2D m_bow;
	Sprite2D m_mouseLeft;

	Menu_PoppingButton m_btn_back;

public:

	ControlState();
	~ControlState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();

};
