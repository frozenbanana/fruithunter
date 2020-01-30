#pragma once
#include "State.hpp"

class IntroState : public State {
public:
	IntroState();
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

private:
};
