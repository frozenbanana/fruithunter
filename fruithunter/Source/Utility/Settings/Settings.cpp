#include "Settings.h"
#include "Renderer.h"

Settings Settings::m_this;

Settings::Settings(){};

void Settings::initialize() {}

Settings* Settings::getInstance() { return &m_this; }

Settings::~Settings() {}

void Settings::setVsync(bool value) { m_vsync = value; }

void Settings::setDarkEdges(bool value) { m_darkEdges = value; }

void Settings::setParticles(bool value) { m_particles = value; }

bool Settings::getVsync() { return m_vsync; }

bool Settings::getDarkEdges() { return m_darkEdges; }

bool Settings::getParticles() { return m_particles; }
