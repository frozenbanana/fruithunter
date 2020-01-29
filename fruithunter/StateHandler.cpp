#include "StateHandler.hpp"
#include "ErrorLogger.hpp"

// Helper function to get current State
State* StateHandler::getCurrent() { return m_states.back(); }

void StateHandler::changeState(State* state) {
	ErrorLogger::log("Current number of states: " + m_states.size());
	// Pause current state
	if (!m_states.empty()) {
		getCurrent()->pause();
		m_states.pop_back();
	}
	// Update current state
	m_states.push_back(state);
	getCurrent()->init();
}

void StateHandler::pushState(State* state) {
	// Pause current state
	if (!m_states.empty()) {
		getCurrent()->pause();
	}

	state->init();
	m_states.push_back(state);
	getCurrent()->play();
}

void StateHandler::popState() {
	if (!m_states.empty()) {
		m_states.pop_back();
		getCurrent()->play();
	}
}

void StateHandler::event(int event) { getCurrent()->handleEvent(event); };

void StateHandler::pause() { getCurrent()->pause(); }

void StateHandler::update() { getCurrent()->update(); }

void StateHandler::play() { getCurrent()->play(); }

void StateHandler::draw() { getCurrent()->draw(); }

bool StateHandler::isRunning() { return m_running; }

void StateHandler::quit() { m_running = false; }
