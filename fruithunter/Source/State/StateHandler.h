#pragma once
#include <vector>
#include <memory>
#include "State.h"

class StateHandler {
public:
	enum States { INTRO, PLAY, PAUSE, SETTINGS, LEVEL_SELECT, ENDROUND, LENGTH };
	void initialize();
	void changeState(States state);
	State* peekState(States state);
	void resumeState();
	void resumeMenuState();
	void handleEvent();
	void pause();
	void update();
	void play();
	void draw();
	bool isRunning();
	void quit();

	static StateHandler* getInstance();
	State* getCurrent();

private:
	bool m_running = true;
	static StateHandler m_this;

	int m_current = INTRO;
	int m_previous = INTRO;
	int m_previousMenu = INTRO;
	std::vector<std::unique_ptr<State>> m_states;

	StateHandler();
};
