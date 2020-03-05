#include "StateHandler.h"
#include "IntroState.h"
#include "PlayState.h"
#include "PauseState.h"
#include "ErrorLogger.h"

StateHandler StateHandler::m_this;

StateHandler::StateHandler() {}

// Helper function to get current State
State* StateHandler::getCurrent() { return m_states[m_current].get(); }

void StateHandler::changeState(States state) {
	ErrorLogger::log("Current state: " + (int)state);
	getCurrent()->pause();
	m_current = state; // update current
	getCurrent()->play();
}

void StateHandler::initialize() {
	m_states.resize(LENGTH);
	m_states[INTRO] = make_unique<IntroState>();
	m_states[PLAY] = make_unique<PlayState>();
	m_states[PAUSE] = make_unique<PauseState>();
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