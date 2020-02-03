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

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	// Quad
	m_quad.draw();

	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("HERE IS THE GOAT", Vector2(400., 300.), col);


	ErrorLogger::log(m_name + " draw() called.");
}

void PlayState::play() {
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
