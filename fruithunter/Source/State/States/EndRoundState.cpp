#include "EndRoundState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"

EndRoundState::EndRoundState() { initialize(); }

EndRoundState::~EndRoundState() {}

void EndRoundState::initialize() {
	m_name = "End Round State";
	m_victoryText = "Nothing";
	m_mainMenuButton.initialize("Main menu", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));
	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 100));
	m_particleSystem = ParticleSystem(ParticleSystem::CONFETTI);
	ErrorLogger::log(
		"confetti initalized, confirmed here: " + to_string(m_particleSystem.getIsActive()));
	m_particleSystem.setPosition(float3(0.0f, -1.f, 0.f));
	m_timer.reset();
	m_camera.setView(float3(0.f, 0.f, 1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));
	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void EndRoundState::update() {
	m_timer.update();
	float dt = m_timer.getDt();
	if (m_resumeButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::PLAY);
	}
	if (m_mainMenuButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::INTRO);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
	m_particleSystem.update(dt, float3(0.f, 0.4f, 0.0f));
}

void EndRoundState::handleEvent() {}

void EndRoundState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void EndRoundState::play() {
	Input::getInstance()->setMouseModeAbsolute();
	ErrorLogger::log(m_name + " play() called.");
}

void EndRoundState::draw() {
	Renderer::getInstance()->beginFrame();

	m_textRenderer.draw(m_timeText, float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 125),
		float4(1., 1.f, 1.f, 1.0f));
	m_textRenderer.draw(
		m_victoryText, float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50), m_victoryColor);
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
