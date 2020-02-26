#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Input.h"

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	// m_apples.resize(16);

	// m_entity.loadAnimated("Bow", 3);
	// m_entity.load("Melon_000000");
	m_camera.setView(float3(0.f, 0.f, -10.f), float3(0.f, 0.f, 0.f), float3(0.f, 1.f, 0.f));
}

void IntroState::update() {
	// AudioHandler::logStats();
	m_timer.update();
	float dt = m_timer.getDt();

	m_entity.updateAnimated(dt);
	/*for (size_t i = 0; i < 16; ++i) {
		float dir = (float)(((int)i % 2) * 2.f) - 1.f;
		float angle = dir * m_timer.getTimePassed() * 0.3f;
		float offset = i * 6.28f / 16.f;
		m_apples[i].updateAnimated(dt);
		float3 appleDestination =
			float3(sin(angle + offset), (i % 2) * 0.5f - 0.3f, cos(angle + offset)) * 10.0f;
		m_apples[i].setNextDestination(appleDestination);
	}*/
	Input::getInstance()->setMouseModeAbsolute();
}

void IntroState::handleEvent() {
	Input* input = Input::getInstance();
	AudioHandler* audioHandler = AudioHandler::getInstance();
	if (input->keyDown(DirectX::Keyboard::B)) {
		audioHandler->playOnce(AudioHandler::Sounds::HEAVY_ARROW);
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void IntroState::draw() {
	m_camera.bindMatrix();
	// ErrorLogger::log(m_name + " draw() called.");
	float t = m_timer.getTimePassed();

	/*if (Input::getInstance()->keyDown(Keyboard::Space))
		m_entity.draw_boundingBox();*/

	/*for (size_t i = 0; i < 16; i++) {
		m_apples[i].draw_animate();
	}*/

	float4 menuColor = float4(0.f, 1.0f, 0.f, 1.0f);
	// m_entity.draw_animate();
	// m_entity.draw();
	m_textRenderer.draw("Main Menu", float2(400., 75.), float4(0.6f, .3f, 0.3f, 1.f));
	m_textRenderer.draw("Play", float2(400., 200.), menuColor);
	m_textRenderer.draw("See Highscore", float2(400., 275.), menuColor);
	m_textRenderer.draw("Settings", float2(400., 350.), menuColor);
	m_textRenderer.draw("Quit", float2(400., 425.), menuColor);
}

void IntroState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	ErrorLogger::log(m_name + " play() called.");
}
