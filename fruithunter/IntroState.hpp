#pragma once
#include "State.hpp"

class IntroState : public State {
public:
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

	static IntroState* getInstance() { return &m_introState; }

protected:
	IntroState() { initialize(); }

private:
	bool m_isLoaded = false;
	static IntroState m_introState;
};
