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
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	Settings* settings = Settings::getInstance();

	m_masterVolume.initialize("Master Volume", float2(width / 2, height / 2 - 250));
	m_musicVolume.initialize("Music Volume", float2(width / 2, height / 2 - 200));
	m_effectsVolume.initialize("Effects Volume", float2(width / 2, height / 2 - 150));
	m_drawDistance.initialize("Draw distance", float2(width / 2, height / 2 - 70));
	m_shadowsButton.initialize(
		"Shadows", float2(width / 2, height / 2 - 10), Button::Setting::MEDIUM);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(width / 2, height / 2 + 50), settings->getDarkEdges());

	m_resolutionButton.initialize(
		"Resolution", float2(width / 2, height / 2 + 140), Button::Resolution::HD);
	m_fullscreenButton.initialize(
		"Fullscreen", float2(width / 2 + 150, height / 2 + 200), settings->getFullscreen());
	m_vsyncButton.initialize(
		"V-Sync", float2(width / 2 - 150, height / 2 + 200), settings->getVsync());

	if (settings->getResolution().x == 1280)
		m_resolutionButton.setResolution(Button::Resolution::HD);
	else if (settings->getResolution().x == 1920)
		m_resolutionButton.setResolution(Button::Resolution::FHD);
	else if (settings->getResolution().x == 2560)
		m_resolutionButton.setResolution(Button::Resolution::QHD);
	else if (settings->getResolution().x == 38400)
		m_resolutionButton.setResolution(Button::Resolution::UHD);

	if (settings->getShadowResolution().x == 1024)
		m_shadowsButton.setLowMedHighUltra(Button::Setting::LOW);
	else if (settings->getShadowResolution().x == 2048)
		m_shadowsButton.setLowMedHighUltra(Button::Setting::MEDIUM);
	else if (settings->getShadowResolution().x == 4096)
		m_shadowsButton.setLowMedHighUltra(Button::Setting::HIGH);
	else if (settings->getShadowResolution().x == 8192)
		m_shadowsButton.setLowMedHighUltra(Button::Setting::ULTRA);

	m_masterVolume.setValue(settings->getMasterVolume());
	m_musicVolume.setValue(settings->getMusicVolume());
	m_effectsVolume.setValue(settings->getEffectsVolume());
	m_drawDistance.setValue(settings->getDrawDistanceValue());

	m_backButton.initialize("Back", float2(width / 2, height - 80));
	m_applyButton.initialize("Apply", float2(width / 2 - 100, height - 80));
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
			settings->setShadowResolution(1024);
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::MEDIUM)
			settings->setShadowResolution(2048);
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::HIGH)
			settings->setShadowResolution(4096);
		else if (m_shadowsButton.getLowMedHighUltra() == Button::Setting::ULTRA)
			settings->setShadowResolution(8192);
	}
	if (m_fullscreenButton.update()) {
		m_screenStateChanged = true;
	}
	if (m_resolutionButton.update()) {
		m_screenStateChanged = true;
	}
	if (m_screenStateChanged) {
		if (m_applyButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Enter)) {
			settings->setFullscreen(m_fullscreenButton.getOnOff());

			if (m_resolutionButton.getResolution() == Button::Resolution::HD)
				settings->setResolution(1280, 720);
			else if (m_resolutionButton.getResolution() == Button::Resolution::FHD)
				settings->setResolution(1920, 1080);
			else if (m_resolutionButton.getResolution() == Button::Resolution::QHD)
				settings->setResolution(2560, 1440);
			else if (m_resolutionButton.getResolution() == Button::Resolution::UHD)
				settings->setResolution(3840, 2160);

			float width = SCREEN_WIDTH;
			float height = SCREEN_HEIGHT;
			m_masterVolume.setPosition(float2(width / 2, height / 2 - 250));
			m_musicVolume.setPosition(float2(width / 2, height / 2 - 200));
			m_effectsVolume.setPosition(float2(width / 2, height / 2 - 150));

			m_drawDistance.setPosition(float2(width / 2, height / 2 - 70));
			m_shadowsButton.setPosition(float2(width / 2, height / 2 - 10));
			m_darkEdgesButton.setPosition(float2(width / 2, height / 2 + 50));

			m_resolutionButton.setPosition(float2(width / 2, height / 2 + 140));
			m_fullscreenButton.setPosition(float2(width / 2 + 150, height / 2 + 200));
			m_vsyncButton.setPosition(float2(width / 2 - 150, height / 2 + 200));

			m_backButton.setPosition(float2(width / 2, height - 80));
			m_applyButton.setPosition(float2(width / 2 - 100, height - 80));

			m_screenStateChanged = false;
		}
	}

	if (m_backButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->resumeMenuState();
	}
}

void SettingsState::pause() { Settings::getInstance()->saveAllSetting(); }

void SettingsState::play() { m_screenStateChanged = false; }

void SettingsState::draw() {
	Renderer::getInstance()->beginFrame();

	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_shadowsButton.draw();
	m_resolutionButton.draw();
	m_fullscreenButton.draw();

	m_drawDistance.draw();
	m_masterVolume.draw();
	m_musicVolume.draw();
	m_effectsVolume.draw();

	if (m_screenStateChanged) {
		m_backButton.setPosition(float2(SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT - 80));
		m_backButton.draw();
		m_applyButton.draw();
	}
	else {
		m_backButton.setPosition(float2(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80));
		m_backButton.draw();
	}

	// Just ignore this. It fixes things
	m_entity.draw();
}
