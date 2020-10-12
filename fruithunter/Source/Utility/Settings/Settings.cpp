#include "Settings.h"
#include "Renderer.h"
#include "AudioHandler.h"
#include "ErrorLogger.h"

#include <fstream>
#include <iostream>
#include <sstream>

Settings Settings::m_this;

Settings::Settings() {};

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
	ifstream file;

	file.open(m_filePath);

	if (file.is_open()) {
		setVsync(stoi(getSetting(&file)));
		setFullscreen(stoi(getSetting(&file)));
		setDarkEdges(stoi(getSetting(&file)));

		setMasterVolume(stof(getSetting(&file)));
		setMusicVolume(stof(getSetting(&file)));
		setEffectsVolume(stof(getSetting(&file)));
		setDrawDistance(stof(getSetting(&file)));

		int x = stoi(getSetting(&file));
		int y = stoi(getSetting(&file));

		setResolution(x, y);
		setShadowResolution(stoi(getSetting(&file)));

		setSensitivity(stof(getSetting(&file)));
	}
	file.close();
}

void Settings::saveAllSetting() {
	ofstream file;
	file.open(m_filePath);

	if (file.is_open()) {
		file << "VerticalSync\t" << m_vsync << "\n";
		file << "Fullscreen\t" << m_fullscreen << "\n";
		file << "DarkEdges\t" << m_darkEdges << "\n";

		file << "MasterVolume\t" << m_masterVolume << "\n";
		file << "MusicVolume\t" << m_musicVolume << "\n";
		file << "EffectsVolume\t" << m_effectsVolume << "\n";
		file << "DrawDistance\t" << m_drawDistance << "\n";

		file << "ResolutionX\t" << m_resolution.x << "\n";
		file << "ResolutionY\t" << m_resolution.y << "\n";
		file << "ShadowRes\t" << m_shadowResolution.x << "\n";
		file << "Sensitivity\t" << m_sensitivity << "\n";

		file.close();
	}
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
	AudioHandler::getInstance()->setMasterVolume(m_masterVolume);
}

void Settings::setMusicVolume(float value) {
	m_musicVolume = value;
	AudioHandler::getInstance()->setMusicVolume(m_musicVolume);
}

void Settings::setEffectsVolume(float value) {
	m_effectsVolume = value;
	AudioHandler::getInstance()->setEffectsVolume(m_effectsVolume);
}

void Settings::setSensitivity(float value) { m_sensitivity = value; }

void Settings::setResolution(int width, int height) {
	XMINT2 desiredResolution(width, height);
	m_resolution = desiredResolution;
	Renderer::getInstance()->changeResolution(width, height);
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
