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

	m_masterVolume.initialize(
		"Master Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 200));
	m_musicVolume.initialize("Music Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 150));
	m_effectsVolume.initialize(
		"Effects Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 100));
	m_drawDistance.initialize(
		"Draw distance", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));


	m_vsyncButton.initialize("V-Sync", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50), true);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 100), true);

	m_backButton.initialize("Back", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT - 100));
	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void SettingsState::update() { Input::getInstance()->setMouseModeAbsolute(); }

void SettingsState::handleEvent() {
	Settings* settings = Settings::getInstance();

	if (m_masterVolume.update()) {
		settings->setMasterVolume(m_masterVolume.getValue());
	}
	if (m_musicVolume.update()) {
		settings->setMusicVolume(m_musicVolume.getValue());
	}
	if (m_effectsVolume.update()) {
		settings->setEffectsVolume(m_effectsVolume.getValue());
	}
	if (m_drawDistance.update()) {
		settings->setMasterVolume(m_masterVolume.getValue());
	}
	if (m_vsyncButton.update()) {
		settings->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_darkEdgesButton.update()) {
		settings->setDarkEdges(m_darkEdgesButton.getOnOff());
	}

	if (m_backButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->resumeMenuState();
	}
}

void SettingsState::pause() {}

void SettingsState::play() {}

void SettingsState::draw() {
	Renderer::getInstance()->beginFrame();

	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_backButton.draw();

	m_drawDistance.draw();
	m_masterVolume.draw();
	m_musicVolume.draw();
	m_effectsVolume.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
