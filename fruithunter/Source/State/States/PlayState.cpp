#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include "Input.h"
#include "StateHandler.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";
	m_shadowMap = make_unique<ShadowMapper>();
}

void PlayState::update() {
	PerformanceTimer::start("PlayState_Update", PerformanceTimer::TimeState::state_average);

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}

	m_timer.update();
	float dt = m_timer.getDt();
	m_levelHandler.update(dt);

	PerformanceTimer::stop();
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void PlayState::draw() {
	PerformanceTimer::start("PlayState_Draw", PerformanceTimer::TimeState::state_average);

	if (1) {
		m_shadowMap.get()->update(m_levelHandler.getPlayerPos()); // not needed?

		if (m_staticShadowNotDrawn) {
			//	Set static shadow map info
			m_shadowMap.get()->bindDSVAndSetNullRenderTargetStatic();
			m_shadowMap.get()->bindCameraMatrix();

			// Draw static shadow map
			m_levelHandler.drawShadowStatic();
			m_staticShadowNotDrawn = false;
		}
		// Set shadow map info
		m_shadowMap.get()->bindDSVAndSetNullRenderTarget();
		m_shadowMap.get()->bindCameraMatrix();

		// Draw shadow map
		m_levelHandler.drawShadowDynamicEntities();
	}

	// Set first person info
	Renderer::getInstance()->beginFrame();
	m_shadowMap.get()->bindVPTMatrix();
	m_shadowMap.get()->bindShadowMap();

	// draw first person
	m_levelHandler.draw();

	// Text
	float t = m_timer.getTimePassed();

	PerformanceTimer::stop();
}

void PlayState::setLevel(int newLevel) { m_currentLevel = newLevel; }

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	Renderer::getInstance()->drawLoading();
	m_levelHandler.loadLevel(m_currentLevel);
}