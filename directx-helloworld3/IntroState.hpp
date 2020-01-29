#pragma once
#include "State.hpp"
#include "Entity.h"

class IntroState : public State {
public:
	void update();
	void init();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

	static IntroState* getInstance() { return &m_introState; }

protected:
	IntroState() {}

private:
	static IntroState m_introState;
	Entity entity;
};
