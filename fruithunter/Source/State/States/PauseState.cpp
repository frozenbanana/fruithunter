#include "PauseState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"

PauseState::PauseState() { initialize(); }

PauseState::~PauseState() {}

void PauseState::initialize() {
	m_name = "Pause State";

	m_resumeButton.initialize("Resume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));
	m_mainMenuButton.initialize("Main menu", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2));
	m_vsyncButton.initialize("V-Sync", float2(300, STANDARD_HEIGHT - 100), true);
	m_darkEdgesButton.initialize(
		"Dark edges", float2(STANDARD_WIDTH - 300, STANDARD_HEIGHT - 100), true);
	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void PauseState::update() {

	if (m_resumeButton.update()) {
		StateHandler::getInstance()->resumeState();
	}
	if (m_mainMenuButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::INTRO);
	}
	if (m_vsyncButton.update()) {
		Renderer::getInstance()->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_darkEdgesButton.update()) {
		Renderer::getInstance()->setDarkEdges(m_darkEdgesButton.getOnOff());
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
}

void PauseState::handleEvent() {}

void PauseState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PauseState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	ErrorLogger::log(m_name + " play() called.");
}

void PauseState::draw() {
	Renderer::getInstance()->beginFrame();

	m_resumeButton.draw();
	m_mainMenuButton.draw();
	m_vsyncButton.draw();
	m_darkEdgesButton.draw();
	m_exitButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
