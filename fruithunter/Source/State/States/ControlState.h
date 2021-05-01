#pragma once
#include "StateItem.h"
#include "Sprite2DAnimation.h"
#include "Menu_PoppingButton.h"
#include "Timer.h"
#include "Text2D.h"

class ControlState : public StateItem {
private:
	Timer m_timer;
	Text2D m_text;
	Sprite2D m_background;

	Sprite2DAnimation m_movementKeys;
	Sprite2DAnimation m_charMovement;
	Sprite2DAnimation m_jumpKey;
	Sprite2DAnimation m_charJump;
	Sprite2DAnimation m_slowMo;
	Sprite2DAnimation m_slowMoKey;
	Sprite2DAnimation m_bow;
	Sprite2DAnimation m_mouseLeft;

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
