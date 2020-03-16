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
		"Master Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 240));
	m_musicVolume.initialize("Music Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 180));
	m_effectsVolume.initialize(
		"Effects Volume", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 120));
	m_drawDistance.initialize(
		"Draw distance", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 60));

	m_shadowsButton.initialize(
		"Shadows", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 20), Button::Setting::MEDIUM);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 80), true);
	m_vsyncButton.initialize("V-Sync", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 140), true);

	m_backButton.initialize("Back", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT - 120));

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
		settings->setDrawDistance(m_drawDistance.getValue());
	}
	if (m_vsyncButton.update()) {
		settings->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_darkEdgesButton.update()) {
		settings->setDarkEdges(m_darkEdgesButton.getOnOff());
	}
	if (m_shadowsButton.update()) {
		if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::LOW)
			Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(XMINT2(1024, 1024));
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::MEDIUM)
			Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(XMINT2(2048, 2048));
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::HIGH)
			Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(XMINT2(4096, 4096));
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::ULTRA)
			Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(XMINT2(8192, 8192));
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
	m_shadowsButton.draw();

	m_drawDistance.draw();
	m_masterVolume.draw();
	m_musicVolume.draw();
	m_effectsVolume.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
