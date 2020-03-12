#include "Settings.h"
#include "Renderer.h"
#include "AudioHandler.h"

Settings Settings::m_this;

Settings::Settings(){};

void Settings::initialize() {}

Settings* Settings::getInstance() { return &m_this; }

Settings::~Settings() {}

void Settings::setVsync(bool value) { m_vsync = value; }

void Settings::setDarkEdges(bool value) { m_darkEdges = value; }

void Settings::setParticles(bool value) { m_particles = value; }

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

bool Settings::getVsync() { return m_vsync; }

bool Settings::getDarkEdges() { return m_darkEdges; }

bool Settings::getParticles() { return m_particles; }

float Settings::getDrawDistance() { return 100.f + 100.f * m_drawDistance; }

float Settings::getMasterVolume() { return m_masterVolume; }

float Settings::getMusicVolume() { return m_musicVolume; }

float Settings::getEffectsVolume() { return m_effectsVolume; }
