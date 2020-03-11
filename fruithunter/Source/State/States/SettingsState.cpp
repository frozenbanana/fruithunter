#include "SettingsState.h"
#include "Renderer.h"
#include "AudioHandler.h"
#include "Input.h"
#include "Statehandler.h"
#include "ErrorLogger.h"
#include "Settings.h"

SettingsState::SettingsState() { initialize(); }

SettingsState::~SettingsState() {}

void SettingsState::initialize() {
	m_name = "Settings State";
	m_vsyncButton.initialize("V-Sync", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50), true);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2), true);
	m_backButton.initialize("Back", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void SettingsState::update() { Input::getInstance()->setMouseModeAbsolute(); }

void SettingsState::handleEvent() {
	Settings* settings = Settings::getInstance();

	if (m_vsyncButton.update()) {
		settings->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_darkEdgesButton.update()) {
		settings->setDarkEdges(m_darkEdgesButton.getOnOff());
	}
	if (m_backButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->resumeMenuState();
	}

	if (Input::getInstance()->keyPressed(Keyboard::Keys::I)) {
		settings->setMasterVolume(min(1.0f, settings->getMasterVolume() + 0.1f));
		ErrorLogger::log("Master volume: " + to_string(settings->getMasterVolume()));
	}
	if (Input::getInstance()->keyPressed(Keyboard::Keys::K)) {
		settings->setMasterVolume(max(0.0f, settings->getMasterVolume() - 0.1f));
		ErrorLogger::log("Master volume: " + to_string(settings->getMasterVolume()));
	}
}

void SettingsState::pause() {}

void SettingsState::play() {}

void SettingsState::draw() {
	Renderer::getInstance()->beginFrame();

	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_backButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
