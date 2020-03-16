#include "EndRoundState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "AudioHandler.h"

EndRoundState::EndRoundState() { initialize(); }

EndRoundState::~EndRoundState() {}

void EndRoundState::initialize() {
	m_name = "End Round State";
	m_victoryText = "Nothing";

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_levelSelectButton.initialize("Select Level", float2(width / 2, height / 2 + 50));
	m_mainMenuButton.initialize("Main Menu", float2(width / 2, height / 2 + 120));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 190));
	m_particleSystem = ParticleSystem(ParticleSystem::CONFETTI);
	m_particleSystem.setPosition(float3(0.0f, -1.f, 0.f));
	m_timer.reset();
	m_camera.setView(float3(0.f, 0.f, 1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));
	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();

	m_particleSystem.update(dt, float3(0.f, 0.4f, 0.0f));
}

void EndRoundState::handleEvent() {
	if (m_levelSelectButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_mainMenuButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->changeState(StateHandler::INTRO);
	}
	if (m_exitButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->quit();
	}
}

void EndRoundState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void EndRoundState::play() {
	ErrorLogger::log(m_name + " play() called.");
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_levelSelectButton.setPosition(float2(width / 2, height / 2 + 50));
	m_mainMenuButton.setPosition(float2(width / 2, height / 2 + 120));
	m_exitButton.setPosition(float2(width / 2, height / 2 + 190));
}

void EndRoundState::draw() {
	Renderer::getInstance()->beginFrame();
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_textRenderer.draw(
		m_timeText, float2(width / 2, height / 2 - 125), float4(1., 1.f, 1.f, 1.0f));
	m_textRenderer.draw(m_victoryText, float2(width / 2, height / 2 - 50), m_victoryColor);
	m_levelSelectButton.draw();
	m_mainMenuButton.draw();
	m_exitButton.draw();
	m_camera.bindMatrix();
	m_particleSystem.draw();
	// Just ignore this. It fixes things
	m_entity.draw();
}
void EndRoundState::setTimeText(string text) { m_timeText = text; }

void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }

void EndRoundState::setVictoryText(string text) { m_victoryText = text; }

void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
