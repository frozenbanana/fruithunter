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

	// Timer
	// TODO: Refactor to a static timeHandler
	LARGE_INTEGER timer;
	if (!QueryPerformanceCounter(&timer)) {
		ErrorLogger::log("Cannot query performance counter in " + m_name + ".");
		return;
	}

	m_frequencySeconds = (float)(timer.QuadPart);
	// Get Current value
	QueryPerformanceCounter(&timer);
	m_startTime = timer.QuadPart;
	m_totalTime = 0.;
	m_elapsedTime = 0.;
}

void PlayState::update() {
	// TODO: Refactor to a static timeHandler
	QueryPerformanceCounter(&m_timer);
	m_elapsedTime = (float)(m_timer.QuadPart - m_startTime);
	m_startTime = m_timer.QuadPart;
	m_totalTime += m_elapsedTime * 0.000001;

	float t = m_totalTime;
	m_camera.setEye(Vector3(sin(t), 0.1 * cos(t), -4.0));
	m_camera.buildMatrices();
	m_camera.updateBuffer();
}

void PlayState::handleEvent(int event) { return; }

void PlayState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PlayState::draw() {
	m_camera.bindMatix();
	m_quad.draw();
	ErrorLogger::log(m_name + " draw() called.");
}

void PlayState::play() { ErrorLogger::log(m_name + " play() called."); }
