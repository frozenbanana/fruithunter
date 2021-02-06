#include "EndRoundState.h"
#include "Renderer.h"
#include "Input.h"
#include "AudioController.h"
#include "SceneManager.h"
#include "SaveManager.h"

EndRoundState::EndRoundState() : StateItem(State::EndRoundState) {}

EndRoundState::~EndRoundState() {}

void EndRoundState::init() {
	m_victoryText = "Nothing";

	float width = 1280;
	float height = 720;
	float2 center(width / 2, height / 2);
	m_restartButton.initialize("Try Again", center + float2(0, 50));
	m_levelSelectButton.initialize("Back To Menu", center + float2(0,120));
	m_particleSystem.load(ParticleSystem::STARS_GOLD, 10);
	// m_particleSystem.setEmitRate(10.f);
	m_particleSystem.setPosition(float3(0.0f, -1.f, 0.f));
	m_timer.reset();
	m_camera.setView(float3(0.f, 0.f, -1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));

	m_bowl.load("Bowl");

	Renderer::getInstance()->captureFrame();

	string sceneName = SceneManager::getScene()->m_sceneName;
	const SceneCompletion* savedData = SaveManager::getProgress(sceneName);
	TimeTargets winGrade = SceneManager::getScene()->getWinGrade();
	if (savedData != nullptr) {
		switch (winGrade) {
		case GOLD:
			setVictoryText("You earned GOLD");
			setVictoryColor(float4(1.0f, 0.85f, 0.0f, 1.0f));
			setConfettiPower(22);
			setBowl(sceneName, (int)GOLD);
			setParticleColorByPrize(GOLD);
			break;
		case SILVER:
			setVictoryText("You earned SILVER");
			setVictoryColor(float4(0.8f, 0.8f, 0.8f, 1.0f));
			setConfettiPower(18);
			setBowl(sceneName, (int)SILVER);
			setParticleColorByPrize(SILVER);
			break;
		case BRONZE:
			setVictoryText("You earned BRONZE");
			setVictoryColor(float4(0.85f, 0.55f, 0.25f, 1.0f));
			setConfettiPower(14);
			setBowl(sceneName, (int)BRONZE);
			setParticleColorByPrize(BRONZE);
			break;
		default:
			setVictoryText("You earned NOTHING");
			setVictoryColor(float4(1.0f, 1.0f, 1.0f, 1.0f));
			setConfettiPower(2);
			setBowl(sceneName, (int)BRONZE);
			setParticleColorByPrize(BRONZE);
			break;
		}
		size_t winTime = SceneManager::getScene()->getTime();
		setTimeText("Time : " + Time2DisplayableString(winTime));
	}
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();
	m_bowl.rotateY(dt * 0.5f);
	m_bowlContent.rotateY(dt * 0.5f);
	m_particleSystem.update(dt);

	if (m_restartButton.update()) {
		SceneManager::getScene()->reset();
		pop(true);
	}
	if (m_levelSelectButton.update()) {
		AudioController::getInstance()->flush();
		pop(State::MainState,false);
	}
}

void EndRoundState::pause() { }

void EndRoundState::play() {
	Renderer::getInstance()->captureFrame();
	float width = 1280;
	float height = 720;

	// Set the correct bowl
	// Bowl material and content are set in Playstate handleEvent(hasWon)
	m_bowl.setPosition(float3(0.0f, 0.0f, 0.0f));
	m_bowlContent.setPosition(m_bowl.getPosition());
	m_camera.setEye(m_bowl.getPosition() + float3(0.f, 0.7f, 1.0f));
	m_camera.setTarget(m_bowl.getPosition() + float3(0.f,0.3f,0.f));

	m_restartButton.setPosition(float2(width / 2, height / 2 + 50));
	m_levelSelectButton.setPosition(float2(width / 2, height / 2 + 120));

	AudioController::getInstance()->play("applause", AudioController::SoundType::Effect);
}

void EndRoundState::restart() {}

void EndRoundState::draw() {
	// Draw to shadowmap
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_camera.getFrustumPoints(0.1f));
	shadowMap->setup_depthRendering();
	m_bowl.draw();
	m_bowlContent.draw();
	
	Renderer::getInstance()->beginFrame();	
	Renderer::getInstance()->drawCapturedFrame();
	Renderer::getInstance()->clearDepth();

	shadowMap->setup_shadowRendering();
	m_camera.bind();

	m_particleSystem.draw();
	m_bowl.draw();
	m_bowlContent.draw();
	Renderer::getInstance()->clearDepth();
	float width = 1280;
	float height = 720;
	m_camera.bind();
	m_textRenderer.setColor(Color(1., 1.f, 1.f, 1.0f));
	m_textRenderer.draw(m_timeText, float2(width / 2, height / 2 - 125));
	m_textRenderer.setColor(m_victoryColor);
	m_textRenderer.draw(m_victoryText, float2(width / 2, height / 2 - 50));
	m_restartButton.draw();
	m_levelSelectButton.draw();

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

void EndRoundState::setBowl(string bowlContentEntityName, int bowlMaterial) {
	if (bowlContentEntityName == "scene0")
		m_bowlContent.load("BowlContent1");
	else if (bowlContentEntityName == "scene1")
		m_bowlContent.load("BowlContent2");
	else if (bowlContentEntityName == "scene2")
		m_bowlContent.load("BowlContent3");

	vector<string> bowlNames = { "bowl_gold", "bowl_silver", "bowl_bronze" };
	m_bowl.load(bowlNames[(int)bowlMaterial]);
}

void EndRoundState::setTimeText(string text) { m_timeText = text; }

void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }

void EndRoundState::setVictoryText(string text) { m_victoryText = text; }

void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
