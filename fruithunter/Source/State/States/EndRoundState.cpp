#include "EndRoundState.h"
#include "Renderer.h"
#include "Input.h"
#include "AudioHandler.h"
#include "SceneManager.h"
#include "SaveManager.h"

EndRoundState::EndRoundState() : StateItem(State::EndRoundState) { }

EndRoundState::~EndRoundState() {}

void EndRoundState::init() {
	m_victoryText = "Nothing";

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_restartButton.initialize("Restart", float2(width / 2, height / 2 + 50));
	m_levelSelectButton.initialize("Select Level", float2(width / 2, height / 2 + 120));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 190));
	m_particleSystem.load(ParticleSystem::STARS_GOLD, 10);
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

	Renderer::getInstance()->captureFrame();

	LevelData savedData =
		SaveManager::getInstance()->getActiveSave()[SceneManager::getScene()->m_utility.levelIndex];
	SaveManager::getInstance()->getAllSaveStates();
	switch (savedData.grade) {
	case GOLD:
		setVictoryText("You earned GOLD");
		setVictoryColor(float4(1.0f, 0.85f, 0.0f, 1.0f));
		setConfettiPower(22);
		setBowlMaterial(SceneManager::getScene()->m_utility.levelIndex, (int)GOLD);
		setParticleColorByPrize(GOLD);
		break;
	case SILVER:
		setVictoryText("You earned SILVER");
		setVictoryColor(float4(0.8f, 0.8f, 0.8f, 1.0f));
		setConfettiPower(18);
		setBowlMaterial(SceneManager::getScene()->m_utility.levelIndex, (int)SILVER);
		setParticleColorByPrize(SILVER);
		break;
	case BRONZE:
		setVictoryText("You earned BRONZE");
		setVictoryColor(float4(0.85f, 0.55f, 0.25f, 1.0f));
		setConfettiPower(14);
		setBowlMaterial(SceneManager::getScene()->m_utility.levelIndex, (int)BRONZE);
		setParticleColorByPrize(BRONZE);
		break;
	default:
		setVictoryText("You earned NOTHING");
		setVictoryColor(float4(1.0f, 1.0f, 1.0f, 1.0f));
		setConfettiPower(2);
		setBowlMaterial(SceneManager::getScene()->m_utility.levelIndex, (int)BRONZE);
		setParticleColorByPrize(BRONZE);
		break;
	}
	setTimeText("Time : " + Time2DisplayableString(savedData.timeOfCompletion));
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();
	m_bowl.rotateY(dt * 0.5f);
	m_bowlContents[m_currentBowlContent].rotateY(dt * 0.5f);
	m_particleSystem.update(dt);

	if (m_restartButton.update()) {
		SceneManager::getScene()->reset();
		pop(true);
	}
	if (m_levelSelectButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		pop(State::LevelSelectState,false);
	}
	if (m_exitButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		pop((State)-1, false);
	}
}

void EndRoundState::pause() { }

void EndRoundState::play() {
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

void EndRoundState::restart() {}

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
	m_camera.bind();

	m_particleSystem.draw();
	m_bowl.draw();
	m_bowlContents[m_currentBowlContent].draw();
	Renderer::getInstance()->clearDepth();
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_camera.bind();
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
		m_particleSystem.setType(ParticleSystem::STARS_GOLD);
		break;
	case 1:
		// silver
		m_particleSystem.setType(ParticleSystem::STARS_SILVER);
		break;
	default:
		// bronze
		m_particleSystem.setType(ParticleSystem::STARS_BRONZE);
		break;
	}
}

void EndRoundState::setBowlMaterial(size_t contentIndex, int bowlMaterial) {
	m_currentBowlContent = contentIndex;
	m_bowl.setCurrentMaterial((int)bowlMaterial);
}

void EndRoundState::setTimeText(string text) { m_timeText = text; }

void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }

void EndRoundState::setVictoryText(string text) { m_victoryText = text; }

void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
