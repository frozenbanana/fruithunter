#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2));
	m_startButton.initialize("Start", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void IntroState::update() {
	PerformanceTimer::start("IntroState_Update", PerformanceTimer::TimeState::state_average);

	if (m_startButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}


	PerformanceTimer::stop();
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
	PerformanceTimer::start("IntroState_Draw", PerformanceTimer::TimeState::state_average);

	Renderer::getInstance()->beginFrame();

	m_startButton.draw();
	m_exitButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();

	PerformanceTimer::stop();
}

void IntroState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	ErrorLogger::log(m_name + " play() called.");
}
