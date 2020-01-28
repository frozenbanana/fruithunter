#pragma once
#include "State.hpp"

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
	IntroState() { init(); }

private:
	static IntroState m_introState;
};
