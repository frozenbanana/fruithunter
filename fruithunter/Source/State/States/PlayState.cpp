#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";
	m_shadowMap = make_unique<ShadowMapper>();
}

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
	if (1) {
		// Set shadow map info
		m_shadowMap.get()->update(m_levelHandler.getPlayerPos());
		m_shadowMap.get()->bindDSVAndSetNullRenderTarget();
		m_shadowMap.get()->bindCameraMatrix();
		// Draw shadow map
		m_levelHandler.drawShadow();
	}

	// Set first person info
	Renderer::getInstance()->beginFrame();
	m_shadowMap.get()->bindVPTMatrix();
	m_shadowMap.get()->bindShadowMap();

	// draw first person
	m_levelHandler.draw();

	// Text
	float t = m_timer.getTimePassed();
}

void PlayState::drawShadow() { m_levelHandler.drawShadow(); }

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	m_levelHandler.loadLevel(0);
}
