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

	m_player.initialize();

	m_bow.loadAnimated("Bow", 3);
	m_bow.setPosition(float3(2.f, 0.f, 0.f));
}

void PlayState::update() {
	m_timer.update();
	float dt = m_timer.getDt();
	m_player.update(dt);
	m_bow.updateAnimated(dt);

	m_apple.updateAnimated(dt);
	float3 appleDestination =
		float3(sin(m_timer.getTimePassed() * 0.6f), 0 /*abs(sin(m_timer.getTimePassed())) * 0.3f*/,
			cos(m_timer.getTimePassed() * 0.6f)) *
		5.0f;
	m_apple.setNextDestination(appleDestination);
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

	// Bow
	m_bow.draw_animate();
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
