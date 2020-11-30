#include "Settings.h"
#include "Renderer.h"
#include "AudioController.h"
#include "ErrorLogger.h"

Settings Settings::m_this;

Settings::Settings() { 
	// bind settings variables to file
	m_settingFile.bind("Vsync:b", &m_vsync);
	m_settingFile.bind("Fullscreen:b", &m_fullscreen);
	m_settingFile.bind("DarkEdges:b", &m_darkEdges);
	m_settingFile.bind("MasterVolume:f", &m_masterVolume);
	m_settingFile.bind("MusicVolume:f", &m_musicVolume);
	m_settingFile.bind("EffectsVolume:f", &m_effectsVolume);
	m_settingFile.bind("DrawDistance:f", &m_drawDistance);
	m_settingFile.bind("ResolutionX:i", &m_resolution.x);
	m_settingFile.bind("ResolutionY:i", &m_resolution.y);
	m_settingFile.bind("ShadowResolution:i", &m_shadowResolution);
	m_settingFile.bind("Sensitivity:f", &m_sensitivity);
}

void Settings::initialize() { getInstance()->loadAllSetting(); }

Settings* Settings::getInstance() { return &m_this; }

Settings::~Settings() {}

void Settings::loadAllSetting() {
	// load from file
	m_settingFile.readFile(m_path);
	// Reset variables in case of overhead.
	// Additional code may need to be executed for a variable!
	setVsync(m_vsync);
	setFullscreen(m_fullscreen);
	setDarkEdges(m_darkEdges);
	setMasterVolume(m_masterVolume);
	setMusicVolume(m_musicVolume);
	setEffectsVolume(m_effectsVolume);
	setDrawDistance(m_drawDistance);
	setResolution(m_resolution.x, m_resolution.y);
	setShadowResolution(m_shadowResolution);
	setSensitivity(m_sensitivity);
}

void Settings::saveAllSetting() {
	// write current settings to file
	m_settingFile.writeFile(m_path);
}

void Settings::setVsync(bool value) { m_vsync = value; }

void Settings::setDarkEdges(bool value) { m_darkEdges = value; }

void Settings::setFullscreen(bool value) {
	m_fullscreen = value;
	if (value) {
		RECT screen;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &screen);
		Settings::getInstance()->setResolution(screen.right, screen.bottom);
	}
	Renderer::getInstance()->setFullscreen(m_fullscreen);
}

void Settings::setDrawDistance(float value) { m_drawDistance = value; }

void Settings::setMasterVolume(float value) {
	m_masterVolume = value;
	AudioController::getInstance()->setMasterVolume(m_masterVolume);
}

void Settings::setMusicVolume(float value) {
	m_musicVolume = value;
	AudioController::getInstance()->setMusicVolume(m_musicVolume);
}

void Settings::setEffectsVolume(float value) {
	m_effectsVolume = value;
	AudioController::getInstance()->setEffectsVolume(m_effectsVolume);
}

void Settings::setSensitivity(float value) { m_sensitivity = value; }

void Settings::setResolution(int width, int height) {
	XMINT2 desiredResolution(width, height);
	m_resolution = desiredResolution;
	Renderer::getInstance()->changeResolution(width, height);
}

void Settings::setShadowResolution(int res) {
	m_shadowResolution = res;
	Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(
		XMINT2(m_shadowResolution, m_shadowResolution));
}

bool Settings::getVsync() { return m_vsync; }

bool Settings::getDarkEdges() { return m_darkEdges; }

bool Settings::getFullscreen() { return m_fullscreen; }

float Settings::getDrawDistance() { return 100.f + 100.f * m_drawDistance; }

float Settings::getDrawDistanceValue() { return m_drawDistance; }

float Settings::getMasterVolume() { return m_masterVolume; }

float Settings::getMusicVolume() { return m_musicVolume; }

float Settings::getEffectsVolume() { return m_effectsVolume; }

float Settings::getSensitivity() { return m_sensitivity; }

XMINT2 Settings::getResolution() { return m_resolution; }

XMINT2 Settings::getShadowResolution() { return XMINT2(m_shadowResolution, m_shadowResolution); }
