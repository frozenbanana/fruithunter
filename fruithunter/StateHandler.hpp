#pragma once
#pragma warning disable C26812 // Prefer enum class over enum
#include <vector>
#include "State.hpp"
#include "IntroState.hpp"
#include "PlayState.hpp"
#include "ErrorLogger.hpp"



class StateHandler {
public:
	enum States { INTRO, PLAY, LENGTH };

	void initialize();
	void changeState(States state);
	// void pushState(State* state);
	// void popState();
	void handleEvent();
	void pause();
	void update();
	void play();
	void draw();
	bool isRunning();
	void quit();

	static StateHandler* getInstance();

private:
	bool m_running = true;
	static StateHandler m_this;
	State* getCurrent();

	int m_current = INTRO;
	std::vector<unique_ptr<State>> m_states;

	StateHandler();
};

#pragma warning restore C26812 // Prefer enum class over enum
