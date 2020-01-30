#include "PlayState.hpp"
#include "ErrorLogger.hpp"
#include "Renderer.hpp"
#include "Quad.hpp"

PlayState PlayState::m_playState;

void PlayState::initialize() {
	if (!m_isLoaded) {
		m_name = "Play State";
		m_quad.init();
		m_camera.setView(Vector3(0.0, 0.0, -1.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
		m_camera.createBuffer();
		m_camera.buildMatrices();
		m_camera.updateBuffer();
		m_isLoaded = true;
	}
}

void PlayState::update() { ErrorLogger::log(m_name + " update() called."); }

void PlayState::handleEvent(int event) { return; }

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() {
	m_camera.bindMatix();
	m_quad.draw();
	ErrorLogger::log(m_name + " draw() called.");
}

void PlayState::play() { ErrorLogger::log(m_name + " play() called."); }
