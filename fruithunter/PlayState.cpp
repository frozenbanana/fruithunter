#include "PlayState.hpp"
#include "ErrorLogger.hpp"
#include "Renderer.hpp"
#include "Quad.hpp"
#include <iostream>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;

void PlayState::initialize() {
	// if (!m_isLoaded) {
	m_name = "Play State";
	m_quad.init();

	m_player.initialize();

	// Timer
	// TODO: Refactor to a static timeHandler
	LARGE_INTEGER timer;
	if (!QueryPerformanceCounter(&timer)) {
		ErrorLogger::log("Cannot query performance counter in " + m_name + ".");
		return;
	}

	// m_frequencySeconds = (float)(timer.QuadPart);
	// Get Current value
	QueryPerformanceCounter(&timer);
	// m_startTime = timer.QuadPart;
	// m_totalTime = 0.;
	// m_elapsedTime = 0.;

	// m_isLoaded = true;
	//}
}

void PlayState::update() {
	m_player.update(0.017f);
	m_apple.updateAnimated();
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();

	// Quad
	m_quad.draw();
	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("HERE IS THE GOAT", Vector2(400., 300.), col);

	// Apple
	m_apple.draw_animate();
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
