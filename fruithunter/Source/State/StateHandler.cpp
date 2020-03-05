#include "StateHandler.h"
#include "IntroState.h"
#include "PlayState.h"
#include "LevelSelectState.h"
#include "PauseState.h"
#include "EndRoundState.h"
#include "ErrorLogger.h"

StateHandler StateHandler::m_this;

StateHandler::StateHandler() {}

// Helper function to get current State
State* StateHandler::getCurrent() { return m_states[m_current].get(); }

void StateHandler::changeState(States state) {
	ErrorLogger::log("Current state: " + (int)state);
	getCurrent()->pause();
	m_previous = m_current;
	m_current = state; // update current
	getCurrent()->play();
}

State* StateHandler::peekState(States state) { return m_states[state].get(); }

void StateHandler::resumeState() {
	ErrorLogger::log("Current state: " + (int)m_previous);
	getCurrent()->pause();
	int temp = m_current;
	m_current = m_previous;
	m_previous = temp; // update current
	getCurrent()->play();
}

void StateHandler::initialize() {
	m_states.resize(LENGTH);
	m_states[INTRO] = make_unique<IntroState>();
	m_states[PLAY] = make_unique<PlayState>();
	m_states[PAUSE] = make_unique<PauseState>();
	m_states[LEVEL_SELECT] = make_unique<LevelSelectState>();
	m_states[ENDROUND] = make_unique<EndRoundState>();
	play();
}

void StateHandler::handleEvent() { getCurrent()->handleEvent(); };

void StateHandler::pause() { getCurrent()->pause(); }

void StateHandler::update() { getCurrent()->update(); }

void StateHandler::play() { getCurrent()->play(); }

void StateHandler::draw() { getCurrent()->draw(); }

bool StateHandler::isRunning() { return m_running; }

void StateHandler::quit() { m_running = false; }

StateHandler* StateHandler::getInstance() { return &m_this; }