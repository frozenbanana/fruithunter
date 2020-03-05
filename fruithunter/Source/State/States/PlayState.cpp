#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include "Input.h"
#include "StateHandler.h"
#include "EndRoundState.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";
	m_shadowMap = make_unique<ShadowMapper>();
}

void PlayState::update() {
	PerformanceTimer::Record record("PlayState_Update", PerformanceTimer::TimeState::state_average);

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}

	m_timer.update();
	float dt = m_timer.getDt();
	m_levelHandler.update(dt);
}

void PlayState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}

	if (m_levelHandler.getHUD().hasWon()) {
		ErrorLogger::log("Changing state to END ROUND!");
		StateHandler::getInstance()->changeState(StateHandler::ENDROUND);
		EndRoundState* endRound =
			dynamic_cast<EndRoundState*>(StateHandler::getInstance()->getCurrent());
		endRound->setTimeText("Time : " + m_levelHandler.getHUD().getTimePassed());
		string vicText = "";
		float4 vicColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		float confettiEmitRate = 6.0f;
		switch (m_levelHandler.getHUD().getPrize()) {
		case GOLD:
			vicText += "You earned GOLD";
			vicColor = float4(1.0f, 0.85f, 0.0f, 1.0f);
			confettiEmitRate = 22.0f;
			break;
		case SILVER:
			vicText += "You earned SILVER";
			vicColor = float4(0.8f, 0.8f, 0.8f, 1.0f);
			confettiEmitRate = 18.0f;

			break;
		case BRONZE:
			vicText += "You earned BRONZE";
			vicColor = float4(0.85f, 0.55f, 0.25f, 1.0f);
			confettiEmitRate = 14.0f;

			break;
		default:
			vicText += "You earned NOTHING";
			vicColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
			confettiEmitRate = 2.0f;

			break;
		}
		endRound->setVictoryText(vicText);
		endRound->setVictoryColor(vicColor);
		endRound->setConfettiPower(confettiEmitRate);
	}
}

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void PlayState::draw() {
	PerformanceTimer::Record record("PlayState_Draw", PerformanceTimer::TimeState::state_average);

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
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	Renderer::getInstance()->drawLoading();
	m_levelHandler.loadLevel(0);
}