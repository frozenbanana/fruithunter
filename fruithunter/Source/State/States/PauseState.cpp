#include "PauseState.h"
#include "Input.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"

PauseState::PauseState() { initialize(); }

PauseState::~PauseState() {}

void PauseState::initialize() {
	m_name = "Pause State";

	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2));
	m_resumeButton.initialize("Resume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void PauseState::update() {
	if (m_exitButton.update()) {
		ErrorLogger::log("Exit");
		StateHandler::getInstance()->quit();
	}
	if (m_resumeButton.update()) {
		ErrorLogger::log("Resume");
		StateHandler::getInstance()->changeState(StateHandler::PLAY);
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

	m_exitButton.draw();
	m_resumeButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
