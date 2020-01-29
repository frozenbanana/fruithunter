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
	bool m_isLoaded = false;
	static IntroState m_introState;
	Entity entity;
};
