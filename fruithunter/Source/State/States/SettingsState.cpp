#include "SettingsState.h"
#include "Renderer.h"
#include "Input.h"
#include "Settings.h"

SettingsState::SettingsState() : StateItem(State::SettingState) {}

SettingsState::~SettingsState() {}

void SettingsState::init() {
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	Settings* settings = Settings::getInstance();

	m_masterVolume.initialize("Master Volume", float2(width / 2, height / 2 - 330));
	m_musicVolume.initialize("Music Volume", float2(width / 2, height / 2 - 280));
	m_effectsVolume.initialize("Effects Volume", float2(width / 2, height / 2 - 230));
	m_sensitivity.initialize("Sensitivity", float2(width / 2, height / 2 - 180));
	m_drawDistance.initialize("Draw distance", float2(width / 2, height / 2 - 130));
	m_shadowsButton.initialize(
		"Shadows", float2(width / 2, height / 2 - 80), Button::Setting::MEDIUM);
	m_darkEdgesButton.initialize(
		"Dark Edges", float2(width / 2, height / 2 - 30), settings->getDarkEdges());

	m_resolutionButton.initialize(
		"Resolution", float2(width / 2, height / 2 + 20), Button::Resolution::HD);
	m_fullscreenButton.initialize(
		"Fullscreen", float2(width / 2, height / 2 + 70), settings->getFullscreen());
	m_vsyncButton.initialize(
		"V-Sync", float2(width / 2, height / 2 + 130), settings->getVsync());
	m_FXAAButton.initialize("FXAA", float2(width / 2, height / 2 + 180), settings->getFXAA());

	m_backButton.initialize("Back", float2(width / 2, height / 2 + 230));
	m_applyButton.initialize("Apply", float2(width / 2 - 100, height / 2 + 230));

	m_settingsBackground.load("melon.png");
	m_settingsBackground.setPosition(float2(1280.f / 2.f, 720.f / 2.f));
	m_settingsBackground.setRotation(3.1415f / 2.f);
	m_settingsBackground.setScale(height / 360.f);
	m_settingsBackground.setAlignment(); // center
}

void SettingsState::update() {
	Input::getInstance()->setMouseModeAbsolute();

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
	if (m_sensitivity.update()) {
		settings->setSensitivity(m_sensitivity.getValue());
	}
	if (m_drawDistance.update()) {
		settings->setDrawDistance(m_drawDistance.getValue());
	}
	if (m_vsyncButton.update()) {
		settings->setVsync(m_vsyncButton.getOnOff());
	}
	if (m_FXAAButton.update()) {
		settings->setFXAA(m_FXAAButton.getOnOff());
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
			m_masterVolume.setPosition(float2(width / 2, height / 2 - 330));
			m_musicVolume.setPosition(float2(width / 2, height / 2 - 280));
			m_effectsVolume.setPosition(float2(width / 2, height / 2 - 230));
			m_sensitivity.setPosition(float2(width / 2, height / 2 - 180));
			m_drawDistance.setPosition(float2(width / 2, height / 2 - 130));

			m_shadowsButton.setPosition(float2(width / 2, height / 2 - 80));
			m_darkEdgesButton.setPosition(float2(width / 2, height / 2 - 30));

			m_resolutionButton.setPosition(float2(width / 2, height / 2 + 20));
			m_fullscreenButton.setPosition(float2(width / 2, height / 2 + 70));
			m_vsyncButton.setPosition(float2(width / 2, height / 2 + 130));
			m_FXAAButton.setPosition(float2(width / 2, height / 2 + 180));

			m_backButton.setPosition(float2(width / 2, height / 2 + 230));
			m_applyButton.setPosition(float2(width / 2 - 100, height / 2 + 230));

			m_screenStateChanged = false;

			m_settingsBackground.setScale(height / 360.f);
		}
	}

	if (m_backButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Escape)) {
		pop(false);
	}
}

void SettingsState::pause() { Settings::getInstance()->saveAllSetting(); }

void SettingsState::play() {
	Settings* settings = Settings::getInstance();

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_masterVolume.setPosition(float2(width / 2, height / 2 - 330));
	m_musicVolume.setPosition(float2(width / 2, height / 2 - 280));
	m_effectsVolume.setPosition(float2(width / 2, height / 2 - 230));
	m_sensitivity.setPosition(float2(width / 2, height / 2 - 180));
	m_drawDistance.setPosition(float2(width / 2, height / 2 - 130));

	m_shadowsButton.setPosition(float2(width / 2, height / 2 - 80));
	m_darkEdgesButton.setPosition(float2(width / 2, height / 2 - 30));

	m_resolutionButton.setPosition(float2(width / 2, height / 2 + 20));
	m_fullscreenButton.setPosition(float2(width / 2, height / 2 + 70));
	m_vsyncButton.setPosition(float2(width / 2, height / 2 + 130));
	m_FXAAButton.setPosition(float2(width / 2, height / 2 + 180));

	m_backButton.setPosition(float2(width / 2, height / 2 + 230));
	m_applyButton.setPosition(float2(width / 2 - 100, height / 2 + 230));

	m_vsyncButton.setOnOff(settings->getVsync());
	m_FXAAButton.setOnOff(settings->getFXAA());
	m_fullscreenButton.setOnOff(settings->getFullscreen());
	m_darkEdgesButton.setOnOff(settings->getDarkEdges());

	if (settings->getResolution().x == 1280)
		m_resolutionButton.setResolution(Button::Resolution::HD);
	else if (settings->getResolution().x == 1920)
		m_resolutionButton.setResolution(Button::Resolution::FHD);
	else if (settings->getResolution().x == 2560)
		m_resolutionButton.setResolution(Button::Resolution::QHD);
	else if (settings->getResolution().x == 3840)
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
	m_sensitivity.setValue(settings->getSensitivity());
	m_drawDistance.setValue(settings->getDrawDistanceValue());

	m_screenStateChanged = false;
}

void SettingsState::restart() {}

void SettingsState::draw() {
	Renderer::getInstance()->beginFrame();
	Renderer::getInstance()->drawCapturedFrame();
	m_settingsBackground.drawNoScaling();
	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_FXAAButton.draw();
	m_shadowsButton.draw();
	m_resolutionButton.draw();
	m_fullscreenButton.draw();

	m_drawDistance.draw();
	m_masterVolume.draw();
	m_musicVolume.draw();
	m_effectsVolume.draw();
	m_sensitivity.draw();

	if (m_screenStateChanged) {
		m_backButton.setPosition(float2(SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 230));
		m_backButton.draw();
		m_applyButton.draw();
	}
	else {
		m_backButton.setPosition(float2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 230));
		m_backButton.draw();
	}
}
