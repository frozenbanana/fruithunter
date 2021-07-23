#include "SettingsState.h"
#include "Renderer.h"
#include "Input.h"
#include "Settings.h"

string SettingsState::screenModeToString(Renderer::ScreenMode mode) {
	static const string screenModeStr[Renderer::ScreenMode::ScreenModeSize] = { "Windowed",
		"Fullscreen", "Borderless" };
	return screenModeStr[mode];
}

SettingsState::SettingsState() : StateItem(State::SettingState) {}

SettingsState::~SettingsState() {}

void SettingsState::init() {
	float width = 1280;
	float height = 720;
	float2 center = float2(1280, 720) / 2.f;

	Settings* settings = Settings::getInstance();

	float2 stepSize = float2(0, 50);
	float2 position = center + float2(0, -290);
	m_masterVolume.initialize("Master Volume", position);
	position += stepSize;
	m_musicVolume.initialize("Music Volume", position);
	position += stepSize;
	m_effectsVolume.initialize("Effects Volume", position);
	position += stepSize;
	m_sensitivity.initialize("Sensitivity", position);
	position += stepSize;
	m_drawDistance.initialize("Draw distance", position);
	position += stepSize;
	m_shadowsButton.initialize("Shadows", position, Button::Setting::MEDIUM);
	position += stepSize;
	m_darkEdgesButton.initialize("Dark Edges", position, settings->getDarkEdges());
	position += stepSize;
	m_resolutionButton.initialize("Resolution", position, Button::Resolution::HD);
	position += stepSize;
	m_screenMode.initialize("Screen Mode: Windowed", position);
	position += stepSize;
	m_vsyncButton.initialize("V-Sync", position, settings->getVsync());
	position += stepSize;
	m_FXAAButton.initialize("FXAA", position, settings->getFXAA());
	position += stepSize;

	m_backButton.initialize("Back", position);
	m_applyButton.initialize("Apply", position + float2(100, 0));

	m_settingsBackground.load("melon.png");
	m_settingsBackground.setPosition(center);
	m_settingsBackground.setRotation(3.1415f / 2.f);
	m_settingsBackground.setScale(2.f);
	m_settingsBackground.setAlignment(); // center
}

void SettingsState::update(double dt) {
	Input* ip = Input::getInstance();
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
	if (m_resolutionButton.update()) {
		m_screenStateChanged = true;
	}
	if (m_screenMode.update()) {
		m_screenModeLocal =
			(Renderer::ScreenMode)((m_screenModeLocal + 1) % Renderer::ScreenMode::ScreenModeSize);
		m_screenStateChanged = true;
	}
	if (m_screenStateChanged) {
		if (m_applyButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Enter)) {

			settings->setScreenMode(m_screenModeLocal);

			if (m_screenModeLocal != Renderer::ScreenMode::Screen_Borderless) {
				if (m_resolutionButton.getResolution() == Button::Resolution::HD)
					settings->setResolution(1280, 720);
				else if (m_resolutionButton.getResolution() == Button::Resolution::FHD)
					settings->setResolution(1920, 1080);
				else if (m_resolutionButton.getResolution() == Button::Resolution::QHD)
					settings->setResolution(2560, 1440);
				else if (m_resolutionButton.getResolution() == Button::Resolution::UHD)
					settings->setResolution(3840, 2160);
			}

			m_screenStateChanged = false;
			m_redraw = true; // will redraw scene at draw call
		}
	}

	if (m_backButton.update() || Input::getInstance()->keyDown(Keyboard::Keys::Escape)) {
		pop(false);
	}
}

void SettingsState::pause() { Settings::getInstance()->saveAllSetting(); }

void SettingsState::play() {
	Settings* settings = Settings::getInstance();

	m_vsyncButton.setOnOff(settings->getVsync());
	m_FXAAButton.setOnOff(settings->getFXAA());
	m_darkEdgesButton.setOnOff(settings->getDarkEdges());

	m_screenModeLocal = Renderer::getInstance()->getScreenMode();

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
	if (m_redraw) {
		m_sceneManager.setup_shadow();
		m_sceneManager.draw_shadow();
		m_sceneManager.setup_color();
		m_sceneManager.draw_color();
		m_sceneManager.draw_finalize();
		m_redraw = false;
	}

	Renderer::getInstance()->drawCapturedFrame();

	m_settingsBackground.draw();
	m_darkEdgesButton.draw();
	m_vsyncButton.draw();
	m_FXAAButton.draw();
	m_shadowsButton.draw();
	m_resolutionButton.draw();

	m_drawDistance.draw();
	m_masterVolume.draw();
	m_musicVolume.draw();
	m_effectsVolume.draw();
	m_sensitivity.draw();

	m_screenMode.setLabel("Screen Mode: " + screenModeToString(m_screenModeLocal));
	m_screenMode.draw();

	float2 center(1280.f / 2, 720.f / 2);
	if (m_screenStateChanged) {
		m_backButton.setPosition(m_FXAAButton.getPosition() + float2(-100, 50));
		m_backButton.draw();
		m_applyButton.setPosition(m_FXAAButton.getPosition() + float2(100, 50));
		m_applyButton.draw();
	}
	else {
		m_backButton.setPosition(m_FXAAButton.getPosition() + float2(0, 50));
		m_backButton.draw();
	}
}
