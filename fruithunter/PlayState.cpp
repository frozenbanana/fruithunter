#include "PlayState.hpp"
#include "ErrorLogger.hpp"
#include "Renderer.hpp"
#include "Quad.hpp"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";
	m_quad.init();
	m_camera.setView(Vector3(0.0, 0.0, -4.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
	m_camera.createBuffer();
	m_camera.buildMatrices();
	m_camera.updateBuffer();
	m_camera.bindMatix();
}

void PlayState::update() {
	m_timer.update();

	float t = m_timer.getTimePassed();

	m_camera.setEye(Vector3(sin(t), 0.1 * cos(t), -4.0));
	m_camera.buildMatrices();
	m_camera.updateBuffer();
}

void PlayState::handleEvent(int event) { return; }

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() {
	m_quad.draw();
	ErrorLogger::log(m_name + " draw() called.");
}

void PlayState::play() { ErrorLogger::log(m_name + " play() called."); }
