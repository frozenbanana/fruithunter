#pragma once
#include <vector>
#include "State.hpp"

class State;

class StateHandler {
public:
	void changeState(State* state);
	void pushState(State* state);
	void popState();
	void event(int event);
	void pause();
	void update();
	void play();
	void draw();
	bool isRunning();
	void quit();

private:
	bool m_running = true;
	State* getCurrent();
	std::vector<State*> m_states;
};
