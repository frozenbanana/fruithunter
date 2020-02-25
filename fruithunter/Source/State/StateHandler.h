#pragma once
#include <vector>
#include <memory>
#include "State.h"


class StateHandler {
public:
	enum States { INTRO, PLAY, LENGTH };
	void initialize();
	void changeState(States state);
	void handleEvent();
	void pause();
	void update();
	void play();
	void draw();
	void drawShadow();
	bool isRunning();
	void quit();

	static StateHandler* getInstance();

private:
	bool m_running = true;
	static StateHandler m_this;
	State* getCurrent();

	int m_current = INTRO;
	std::vector<std::unique_ptr<State>> m_states;

	StateHandler();
};
