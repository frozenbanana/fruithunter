#include "SettingsState.h"
#include "Renderer.h"
#include "AudioHandler.h"
#include "Input.h"
#include "Statehandler.h"
#include "ErrorLogger.h"

SettingsState::SettingsState() { initialize(); }

SettingsState::~SettingsState() {}

void SettingsState::initialize() {
	ErrorLogger::log("Settings Init");
	m_name = "Settings State";
	m_vsyncButton.initialize("V-Sync", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50), true);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2), true);
	m_backButton.initialize("Back", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void SettingsState::update() { ErrorLogger::log("Settings Update"); }

void SettingsState::handleEvent() {
	ErrorLogger::log("Settings Handle Event");
	if (m_vsyncButton.update()) {
		Renderer::getInstance()->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_darkEdgesButton.update()) {
		Renderer::getInstance()->setDarkEdges(m_darkEdgesButton.getOnOff());
	}
	if (m_backButton.update()) {
		StateHandler::getInstance()->resumeMenuState();
	}
}

void SettingsState::pause() { ErrorLogger::log("Settings Pause"); }

void SettingsState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	ErrorLogger::log("Settings Play");
}

void SettingsState::draw() {
	Renderer::getInstance()->beginFrame();
	ErrorLogger::log("Settings Draw");
	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_backButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
