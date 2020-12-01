#include "Settings.h"
#include "Renderer.h"
#include "AudioController.h"
#include "ErrorLogger.h"

Settings Settings::m_this;

Settings::Settings() { 
	// bind settings variables to file
	m_settingFile.bind("Vsync:b", &m_tempContainer.vsync);
	m_settingFile.bind("Fullscreen:b", &m_tempContainer.fullscreen);
	m_settingFile.bind("DarkEdges:b", &m_tempContainer.darkEdges);
	m_settingFile.bind("MasterVolume:f", &m_tempContainer.volume_master);
	m_settingFile.bind("MusicVolume:f", &m_tempContainer.volume_music);
	m_settingFile.bind("EffectsVolume:f", &m_tempContainer.volume_effect);
	m_settingFile.bind("DrawDistance:f", &m_tempContainer.drawDistance);
	m_settingFile.bind("ResolutionX:i", &m_tempContainer.resolution.x);
	m_settingFile.bind("ResolutionY:i", &m_tempContainer.resolution.y);
	m_settingFile.bind("ShadowResolution:i", &m_tempContainer.resolution_shadow);
	m_settingFile.bind("Sensitivity:f", &m_tempContainer.sensitivity);
}

void Settings::initialize() { getInstance()->loadAllSetting(); }

Settings* Settings::getInstance() { return &m_this; }

Settings::~Settings() {}

void Settings::loadAllSetting() {
	// load from file
	m_settingFile.readFile(m_path);
	// update variables in case of overhead.
	// Additional code may need to be executed for a variable!
	setVsync(m_tempContainer.vsync);
	setFullscreen(m_tempContainer.fullscreen);
	setDarkEdges(m_tempContainer.darkEdges);
	setMasterVolume(m_tempContainer.volume_master);
	setMusicVolume(m_tempContainer.volume_music);
	setEffectsVolume(m_tempContainer.volume_effect);
	setDrawDistance(m_tempContainer.drawDistance);
	setResolution(m_tempContainer.resolution.x, m_tempContainer.resolution.y);
	setShadowResolution(m_tempContainer.resolution_shadow);
	setSensitivity(m_tempContainer.sensitivity);
}

void Settings::saveAllSetting() {
	// write current settings to file
	m_tempContainer = m_container; // copy current settings into temporary container
	m_settingFile.writeFile(m_path);
}

void Settings::setVsync(bool value) { 
	if (m_container.vsync != value) {
		m_container.vsync = value;
	}
}

void Settings::setDarkEdges(bool value) { 
	if (m_container.darkEdges != value) {
		m_container.darkEdges = value;
	}
}

void Settings::setFullscreen(bool value) {
	if (m_container.fullscreen != value) {
		m_container.fullscreen = value;
		Renderer::getInstance()->setFullscreen(m_container.fullscreen);
	}
}

void Settings::setDrawDistance(float value) { 
	if (m_container.drawDistance != value) {
		m_container.drawDistance = value;
	}
}

void Settings::setMasterVolume(float value) {
	if (m_container.volume_master != value) {
		m_container.volume_master = value;
		AudioController::getInstance()->setMasterVolume(m_container.volume_master);
	}
}

void Settings::setMusicVolume(float value) {
	if (m_container.volume_music != value) {
		m_container.volume_music = value;
		AudioController::getInstance()->setMusicVolume(m_container.volume_music);
	}
}

void Settings::setEffectsVolume(float value) {
	if (m_container.volume_effect != value) {
		m_container.volume_effect = value;
		AudioController::getInstance()->setEffectsVolume(m_container.volume_effect);
	}
}

void Settings::setSensitivity(float value) {
	if (m_container.sensitivity != value) {
		m_container.sensitivity = value;
	}
}

void Settings::setResolution(int width, int height) {
	XMINT2 desiredResolution(width, height);
	if (desiredResolution.x != m_container.resolution.x || desiredResolution.y != m_container.resolution.y) {
		m_container.resolution = desiredResolution;
		Renderer::getInstance()->changeResolution(width, height);
	}
}

void Settings::setShadowResolution(int res) {
	if (m_container.resolution_shadow != res) {
		m_container.resolution_shadow = res;
		Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(XMINT2(res, res));
	}
}

bool Settings::getVsync() { return m_container.vsync; }

bool Settings::getDarkEdges() { return m_container.darkEdges; }

bool Settings::getFullscreen() { return m_container.fullscreen; }

float Settings::getDrawDistance() { return 100.f + 100.f * m_container.drawDistance; }

float Settings::getDrawDistanceValue() { return m_container.drawDistance; }

float Settings::getMasterVolume() { return m_container.volume_master; }

float Settings::getMusicVolume() { return m_container.volume_music; }

float Settings::getEffectsVolume() { return m_container.volume_effect; }

float Settings::getSensitivity() { return m_container.sensitivity; }

XMINT2 Settings::getResolution() { return m_container.resolution; }

XMINT2 Settings::getShadowResolution() {
	return XMINT2(m_container.resolution_shadow, m_container.resolution_shadow);
}
