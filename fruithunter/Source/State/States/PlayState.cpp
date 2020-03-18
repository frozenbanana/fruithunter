#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include "Input.h"
#include "StateHandler.h"
#include "EndRoundState.h"
#include "Settings.h"

void PlayState::initialize() { m_name = "Play State"; }

void PlayState::update() {
	Input::getInstance()->setMouseModeRelative();

	m_timer.update();
	float dt = m_timer.getDt();
	m_levelHandler->update(dt);
}

void PlayState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}

	if (m_levelHandler->getHUD().hasWon()) {
		ErrorLogger::log("Changing state to END ROUND.");
		Renderer::getInstance()->captureFrame();
		StateHandler::getInstance()->changeState(StateHandler::ENDROUND);
		EndRoundState* endRound =
			dynamic_cast<EndRoundState*>(StateHandler::getInstance()->getCurrent());
		endRound->setTimeText("Time : " + m_levelHandler->getHUD().getTimePassed());
		string vicText = "";
		float4 vicColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		float confettiEmitRate = 6.0f;
		size_t prizeIndex = 0;
		switch (m_levelHandler->getHUD().getPrize()) {
		case GOLD:
			vicText += "You earned GOLD";
			vicColor = float4(1.0f, 0.85f, 0.0f, 1.0f);
			confettiEmitRate = 22.0f;
			prizeIndex = GOLD;
			break;
		case SILVER:
			vicText += "You earned SILVER";
			vicColor = float4(0.8f, 0.8f, 0.8f, 1.0f);
			confettiEmitRate = 18.0f;
			prizeIndex = SILVER;

			break;
		case BRONZE:
			vicText += "You earned BRONZE";
			vicColor = float4(0.85f, 0.55f, 0.25f, 1.0f);
			confettiEmitRate = 14.0f;
			prizeIndex = BRONZE;
			break;
		default:
			vicText += "You earned NOTHING";
			vicColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
			confettiEmitRate = 2.0f;
			prizeIndex = BRONZE;
			break;
		}
		endRound->setVictoryText(vicText);
		endRound->setVictoryColor(vicColor);
		endRound->setConfettiPower(confettiEmitRate);
		endRound->setBowlMaterial(m_currentLevel, prizeIndex); // change to reflect actual level
		endRound->setParticleColorByPrize(prizeIndex);
	}
}

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_levelHandler->getPlayerFrustumPoints(0.4f));
	shadowMap->setup_depthRendering();

	m_levelHandler->drawShadowDynamic();

	// Set first person info
	Renderer::getInstance()->beginFrame();

	shadowMap->setup_shadowsRendering();

	// draw first person
	m_levelHandler->draw();

	// Text
	float t = m_timer.getTimePassed();
}

void PlayState::setLevel(int newLevel) { m_currentLevel = newLevel; }

void PlayState::destroyLevel() {
	int blob = 0;
	m_levelHandler.reset();
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	if (m_levelHandler == nullptr)
		m_levelHandler = make_unique<LevelHandler>();
	m_levelHandler->loadLevel(m_currentLevel);
	// m_shadowMap->clearAllShadows();
}