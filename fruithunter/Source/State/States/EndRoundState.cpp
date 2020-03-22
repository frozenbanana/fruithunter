#include "EndRoundState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "AudioHandler.h"
#include "PlayState.h"

EndRoundState::EndRoundState() { initialize(); }

EndRoundState::~EndRoundState() {}

void EndRoundState::initialize() {
	m_name = "End Round State";
	m_victoryText = "Nothing";

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_restartButton.initialize("Restart", float2(width / 2, height / 2 + 50));
	m_levelSelectButton.initialize("Select Level", float2(width / 2, height / 2 + 120));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 190));
	m_particleSystem = ParticleSystem(ParticleSystem::STARS);
	// m_particleSystem.setEmitRate(10.f);
	m_particleSystem.setPosition(float3(0.0f, -1.f, 0.f));
	m_timer.reset();
	m_camera.setView(float3(0.f, 0.f, -1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));

	m_background.load("banana.png");
	m_background.setScale(1.40f);
	m_background.setRotation(-0.5f);
	m_background.setPosition(float2((width / 2.5f), (height / 2.0f) - 10.f));

	m_bowl.load("Bowl");
	m_bowlContents[0].load("BowlContent1");
	m_bowlContents[1].load("BowlContent2");
	m_bowlContents[2].load("BowlContent3");

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();
	 m_bowl.rotateY(dt * 0.5f);
	 m_bowlContents[m_currentBowlContent].rotateY(dt * 0.5f);
	m_particleSystem.update(dt, float3(0.f, 0.4f, 0.0f));
}

void EndRoundState::handleEvent() {
	if (m_restartButton.update()) {
		State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
		dynamic_cast<PlayState*>(tempPointer)->destroyLevel();
		TerrainManager::getInstance()->removeAll();
		StateHandler::getInstance()->changeState(StateHandler::PLAY);
	}
	if (m_levelSelectButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_exitButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->quit();
	}
}

void EndRoundState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void EndRoundState::play() {
	ErrorLogger::log(m_name + " play() called.");
	Renderer::getInstance()->captureFrame();
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	// Set the correct bowl
	// Bowl material and content are set in Playstate handleEvent(hasWon)
	m_bowl.setPosition(float3(0.0f, 0.0f, 0.0f));
	m_bowlContents[m_currentBowlContent].setPosition(m_bowl.getPosition());
	m_camera.setEye(m_bowl.getPosition() + float3(0.f, 0.7f, 1.0f));
	m_camera.setTarget(m_bowl.getPosition() + float3(0.f,0.3f,0.f));

	m_restartButton.setPosition(float2(width / 2, height / 2 + 50));
	m_levelSelectButton.setPosition(float2(width / 2, height / 2 + 120));
	m_exitButton.setPosition(float2(width / 2, height / 2 + 190));

	AudioHandler::getInstance()->playOnce(AudioHandler::APPLAUSE);
}

void EndRoundState::draw() {
	// Draw to shadowmap
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_camera.getFrustumPoints(0.1f));
	shadowMap->setup_depthRendering();
	m_bowl.draw();
	m_bowlContents[m_currentBowlContent].draw();
	
	Renderer::getInstance()->beginFrame();	
	Renderer::getInstance()->drawCapturedFrame();
	Renderer::getInstance()->clearDepth();

	shadowMap->setup_shadowRendering();
	m_camera.bindMatrix();

	m_particleSystem.draw();
	m_bowl.draw();
	m_bowlContents[m_currentBowlContent].draw();
	Renderer::getInstance()->clearDepth();
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_camera.bindMatrix();
	//m_background.draw();
	m_textRenderer.draw(
		m_timeText, float2(width / 2, height / 2 - 125), float4(1., 1.f, 1.f, 1.0f));
	m_textRenderer.draw(m_victoryText, float2(width / 2, height / 2 - 50), m_victoryColor);
	m_restartButton.draw();
	m_levelSelectButton.draw();
	m_exitButton.draw();

	
	// Just ignore this. It fixes things
	m_entity.draw();
}

void EndRoundState::setParticleColorByPrize(size_t prize) {
	float4 colors[3];
	switch (prize) {
	case 0:
		// gold
		colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
		colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
		colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
		break;
	case 1:
		// silver
		colors[0] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		colors[1] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		colors[2] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		break;
	default:
		// bronze
		colors[0] = float4(0.69f, 0.34f, 0.05f, 1.0f);
		colors[1] = float4(0.71f, 0.36f, 0.07f, 1.0f);
		colors[2] = float4(0.70f, 0.32f, 0.09f, 1.0f);
		break;
	}

	m_particleSystem.setColors(colors);
}

void EndRoundState::setBowlMaterial(size_t contentIndex, int bowlMaterial) {
	m_currentBowlContent = contentIndex;
	m_bowl.setCurrentMaterial((int)bowlMaterial);
}

void EndRoundState::setTimeText(string text) { m_timeText = text; }

void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }

void EndRoundState::setVictoryText(string text) { m_victoryText = text; }

void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
