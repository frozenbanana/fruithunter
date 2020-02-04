#include "IntroState.hpp"
#include "ErrorLogger.hpp"
#include <SimpleMath.h>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;

IntroState::IntroState()
{
	initialize();
}

IntroState::~IntroState()
{
}

void IntroState::initialize()
{
	m_name = "Intro State";

	m_entity.load("bow");

	m_camera.setView(Vector3(0.0, 0.0, -10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
}

void IntroState::update() {
	m_timer.update();
	float rotSpeed = 1;
	rot += 0.01;
	m_entity.setPosition(float3(5 * sin(rot), 0, 0));
	m_entity.rotateY(3.14f * 1.f / 60.f);
	m_entity.setScale(sin(rot));

	m_camera.updateBuffer();

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
	Vector4 col = Vector4(abs(sin(t)), .5f, abs(cos(t)), 1.f);
	m_textRenderer.draw("LET ME SEE THE GOAT " + std::to_string(t), Vector2(400., 300.), col);

	if (Input::getInstance()->keyDown(Keyboard::Space))
		m_entity.draw_boundingBox();
	m_entity.draw();
}

void IntroState::play() {
	AudioHandler::startMenuAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
