#include "IntroState.hpp"
#include "ErrorLogger.hpp"
#include <SimpleMath.h>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	m_apples.resize(10);

	m_entity.loadAnimated("Bow", 3);

	m_camera.setView(Vector3(0.f, 0.f, -10.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
}

void IntroState::update() {
	// AudioHandler::logStats();
	m_timer.update();
	float dt = m_timer.getDt();

	m_entity.updateAnimated(dt);
	m_camera.updateBuffer();
	for (size_t i = 0; i < 10; i++) {
		m_apples[i].updateAnimated(dt);
	}
	Input::getInstance()->setMouseModeAbsolute();
}

void IntroState::handleEvent() {
	Input* input = Input::getInstance();
	AudioHandler* audioHandler = AudioHandler::getInstance();
	if (input->keyDown(DirectX::Keyboard::B)) {
		ErrorLogger::log("B pressed.");
		audioHandler->playOneTime(AudioHandler::Sounds::LALA);
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void IntroState::draw() {
	m_camera.bindMatrix();
	// ErrorLogger::log(m_name + " draw() called.");
	float t = m_timer.getTimePassed();
	float a = m_timer.getDt();
	// ErrorLogger::log(std::to_string(a));

	if (Input::getInstance()->keyDown(Keyboard::Space))
		m_entity.draw_boundingBox();

	for (size_t i = 0; i < 10; i++) {
		m_apples[i].draw_animate();
	}

	Vector4 menuColor = Vector4(0.f, 1.0f, 0.f, 1.0f);
	m_entity.draw_animate();
	m_textRenderer.draw("Main Menu", Vector2(400., 75.), Vector4(0.6f, .3f, 0.3f, 1.f));
	m_textRenderer.draw("Play", Vector2(400., 200.), menuColor);
	m_textRenderer.draw("See Highscore", Vector2(400., 275.), menuColor);
	m_textRenderer.draw("Settings", Vector2(400., 350.), menuColor);
	m_textRenderer.draw("Quit", Vector2(400., 425.), menuColor);
}

void IntroState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	AudioHandler::startMenuAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
