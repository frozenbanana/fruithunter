#include "IntroState.hpp"
#include "ErrorLogger.hpp"

IntroState IntroState::m_introState;

void IntroState::init() { m_name = "Intro State"; }

void IntroState::update() { ErrorLogger::log(m_name + " update() called."); }

void IntroState::handleEvent(int event) { return; }

void IntroState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void IntroState::draw() { /*ErrorLogger::log(m_name + " draw() called.");*/
}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
