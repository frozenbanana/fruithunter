#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

void PlayState::initialize() { m_name = "Play State"; }

void PlayState::update() {
	m_timer.update();
	float dt = m_timer.getDt();
	m_levelHandler.update(dt);
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void PlayState::draw() {
	m_levelHandler.draw();

	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
}

void PlayState::drawShadow() { m_levelHandler.drawShadow(); }

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	m_levelHandler.loadLevel(0);
}
