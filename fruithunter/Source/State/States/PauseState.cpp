#include "PauseState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "AudioHandler.h"
#include "TerrainManager.h"
#include "PlayState.h"

PauseState::PauseState() { initialize(); }

PauseState::~PauseState() {}

void PauseState::initialize() {
	m_name = "Pause State";

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	m_restartButton.initialize("Restart", float2(width / 2, height / 2 - 120));
	m_resumeButton.initialize("Resume", float2(width / 2, height / 2 - 60));
	m_settingsButton.initialize("Settings", float2(width / 2, height / 2));
	m_mainMenuButton.initialize("Main Menu", float2(width / 2, height / 2 + 60));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 120));

	m_settingsBackground.load("apple.png");
	m_settingsBackground.setPosition(float2(width / 2.f, height / 2.f));
	m_settingsBackground.setScale(1.25f);

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void PauseState::update() { Input::getInstance()->setMouseModeAbsolute(); }

void PauseState::handleEvent() {
	if (m_resumeButton.update() || Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->resumeState();
	}
	if (StateHandler::getInstance()->getPreviousState() == StateHandler::PLAY &&
		m_restartButton.update()) {
		State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
		dynamic_cast<PlayState*>(tempPointer)->destroyLevel();
		TerrainManager::getInstance()->removeAll();
		StateHandler::getInstance()->changeState(StateHandler::PLAY);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_mainMenuButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->changeState(StateHandler::INTRO);
	}
	if (m_exitButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->quit();
	}
}

void PauseState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PauseState::play() {
	ErrorLogger::log(m_name + " play() called.");
	int previousState = StateHandler::getInstance()->getPreviousState();
	if (previousState == StateHandler::LEVEL_SELECT || previousState == StateHandler::PLAY)
		Renderer::getInstance()->captureFrame();
}

float width = SCREEN_WIDTH;
float height = SCREEN_HEIGHT;

m_restartButton.setPosition(float2(width / 2, height / 2 - 120));
m_resumeButton.setPosition(float2(width / 2, height / 2 - 60));
m_settingsButton.setPosition(float2(width / 2, height / 2));
m_mainMenuButton.setPosition(float2(width / 2, height / 2 + 60));
m_exitButton.setPosition(float2(width / 2, height / 2 + 120));
}

void PauseState::draw() {
	Renderer::getInstance()->beginFrame();
	Renderer::getInstance()->drawCapturedFrame();
	m_settingsBackground.draw();

	if (StateHandler::getInstance()->getPreviousState() == StateHandler::PLAY)
		m_restartButton.draw();

	m_resumeButton.draw();
	m_mainMenuButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
