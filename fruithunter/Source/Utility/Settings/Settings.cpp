#include "Settings.h"
#include "Renderer.h"
#include "AudioController.h"
#include "ErrorLogger.h"

#include <fstream>
#include <iostream>
#include <sstream>

Settings Settings::m_this;

Settings::Settings() { 
	m_settingFile.bind("Vsync:b", &m_vsync);
	m_settingFile.bind("Fullscreen:b", &m_fullscreen);
	m_settingFile.bind("DarkEdges:b", &m_darkEdges);
	m_settingFile.bind("MasterVolume:f", &m_masterVolume);
	m_settingFile.bind("MusicVolume:f", &m_musicVolume);
	m_settingFile.bind("EffectsVolume:f", &m_effectsVolume);
	m_settingFile.bind("DrawDistance:f", &m_drawDistance);
	m_settingFile.bind("ResolutionX:i", &m_resolutionX);
	m_settingFile.bind("ResolutionY:i", &m_resolutionY);
	m_settingFile.bind("ShadowResolution:i", &m_shadowResolutionSize);
	m_settingFile.bind("Sensitivity:f", &m_sensitivity);
}

string Settings::getSetting(ifstream* input) {
	string in, word;

	getline(*input, in);
	istringstream line(in);

	getline(line, word, '\t');
	getline(line, word, '\t');

	return word;
}

void Settings::initialize() { getInstance()->loadAllSetting(); }

Settings* Settings::getInstance() { return &m_this; }

Settings::~Settings() {}

void Settings::loadAllSetting() {
	m_settingFile.readFile(m_path);
	setFullscreen(m_fullscreen);
	setMasterVolume(m_masterVolume);
	setMusicVolume(m_musicVolume);
	setEffectsVolume(m_effectsVolume);
	setResolution(m_resolutionX, m_resolutionY);
	setShadowResolution(m_shadowResolutionSize);
}

void Settings::saveAllSetting() {
	m_resolutionX = m_resolution.x;
	m_resolutionY = m_resolution.y;
	m_shadowResolutionSize = m_shadowResolution.x;
	m_settingFile.writeFile(m_path);
}

void Settings::setVsync(bool value) { m_vsync = value; }

void Settings::setDarkEdges(bool value) { m_darkEdges = value; }

void Settings::setFullscreen(bool value) {
	m_fullscreen = value;
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
	if (desiredResolution.x != m_resolution.x || desiredResolution.y != m_resolution.y) {
		m_resolution = desiredResolution;
		Renderer::getInstance()->changeResolution(width, height);
	}
}

void Settings::setShadowResolution(int res) {
	m_shadowResolution = XMINT2(res, res);
	Renderer::getInstance()->getShadowMapper()->resizeShadowDepthViews(m_shadowResolution);
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

XMINT2 Settings::getShadowResolution() { return m_shadowResolution; }
