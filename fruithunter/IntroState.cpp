#include "IntroState.hpp"
#include "ErrorLogger.hpp"
#include <SimpleMath.h>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;

IntroState::IntroState() { initialize(); }

void IntroState::initialize() { m_name = "Intro State"; }

void IntroState::update() {
	m_timer.update();
	// float t = m_timer.getTimePassed();

	// DirectX::XMVECTORF32 col = { abs(sin(t)), .5f, abs(cos(t)), 1.f };
	// m_textRenderer.setColor(col);
	// ErrorLogger::log(m_name + " update() called. " + std::to_string(t));
}

void IntroState::handleEvent(int event) { return; }

void IntroState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void IntroState::draw() {
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(abs(sin(t)), .5f, abs(cos(t)), 1.f);
	m_textRenderer.draw("LET ME SEE THE GOAT", Vector2(400., 300.), col);
	ErrorLogger::log(m_name + " draw() called.");
}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
