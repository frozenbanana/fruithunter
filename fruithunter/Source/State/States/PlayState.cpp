#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include "Input.h"
#include "StateHandler.h"
#include "EndRoundState.h"
#include "Settings.h"
#include "SaveManager.h"

void PlayState::initialize() { m_name = "Play State"; }

void PlayState::update() {
	Input::getInstance()->setMouseModeRelative();
	sceneManager.update();
}

void PlayState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}

	if (SceneManager::getScene()->handleWin()) {
		ErrorLogger::log("Changing state to END ROUND.");
		Renderer::getInstance()->captureFrame();
		LevelData savedData = SaveManager::getInstance()->getActiveSave()[SceneManager::getScene()->m_utility.levelIndex];
		StateHandler::getInstance()->changeState(StateHandler::ENDROUND);
		EndRoundState* endRound =
			dynamic_cast<EndRoundState*>(StateHandler::getInstance()->getCurrent());
		endRound->setTimeText("Time : " + Time2DisplayableString(savedData.timeOfCompletion));
		string vicText = "";
		float4 vicColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		float confettiEmitRate = 6.0f;
		size_t prizeIndex = 0;
		SaveManager::getInstance()->getAllSaveStates();
		switch (savedData.grade) {
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
		endRound->setBowlMaterial(SceneManager::getScene()->m_utility.levelIndex,
			(int)prizeIndex); // change to reflect actual level
		endRound->setParticleColorByPrize(prizeIndex);
	}
}

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() {
	sceneManager.setup_shadow();
	sceneManager.draw_shadow();
	sceneManager.setup_color();
	sceneManager.draw_color();
	sceneManager.draw_hud();
}

void PlayState::changeScene(string sceneName) { sceneManager.load(sceneName); }

void PlayState::restart() { sceneManager.reset(); }

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
}