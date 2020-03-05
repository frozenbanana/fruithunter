#pragma once
#include <vector>
#include <memory>
#include "State.h"

class StateHandler {
public:
	enum States { INTRO, PLAY, PAUSE, LEVEL_SELECT, ENDROUND, LENGTH };
	void initialize();
	void changeState(States state);
	void resumeState();
	void handleEvent();
	void pause();
	void update();
	void play();
	void draw();
	bool isRunning();
	void quit();

	static StateHandler* getInstance();
	State* getCurrent();

	State* getCurrent();

private:
	bool m_running = true;
	static StateHandler m_this;

	int m_current = INTRO;
	int m_previous = INTRO;
	std::vector<std::unique_ptr<State>> m_states;

	StateHandler();
};
