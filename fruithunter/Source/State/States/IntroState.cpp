#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	m_startButton.initialize("Start", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));
	m_settingsButton.initialize("Settings", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2));
	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void IntroState::update() { Input::getInstance()->setMouseModeAbsolute(); }

void IntroState::handleEvent() {
	if (m_startButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void IntroState::draw() {
	// Set first person info
	Renderer::getInstance()->beginFrame();

	m_startButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
