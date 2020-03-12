#include "SettingsState.h"
#include "Renderer.h"
#include "AudioHandler.h"
#include "Input.h"
#include "Statehandler.h"
#include "ErrorLogger.h"

SettingsState::SettingsState() { initialize(); }

SettingsState::~SettingsState() {}

void SettingsState::initialize() {
	m_name = "Settings State";
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_vsyncButton.initialize("V-Sync", float2(width / 2, height / 2 - 50), true);
	m_darkEdgesButton.initialize("Dark Edges", float2(width / 2, height / 2), true);
	m_backButton.initialize("Back", float2(width / 2, height / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void SettingsState::update() { Input::getInstance()->setMouseModeAbsolute(); }

void SettingsState::handleEvent() {

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
