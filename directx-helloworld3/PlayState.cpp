#include "PlayState.hpp"
#include "ErrorLogger.hpp"

PlayState PlayState::m_playState;
void PlayState::init() { m_name = "Play State"; }

void PlayState::update() { ErrorLogger::log(m_name + " update() called."); }

void PlayState::handleEvent(int event) { return; }

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() { ErrorLogger::log(m_name + " draw() called."); }

void PlayState::play() { ErrorLogger::log(m_name + " play() called."); }
