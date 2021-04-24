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

	m_particleSystem.load(ParticleSystem::STARS_GOLD, 10);
	m_timer.reset();
	m_camera.setView(float3(0.f, 0.f, -1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));

	m_spr_background.load("square_white.png");
	m_spr_background.setPosition(center);
	m_spr_background.setColor(float4(0, 0, 0, 1));
	m_spr_background.setAlpha(0.75);
	m_spr_background.setSize(float2(width*0.8, height*1));
	m_spr_background.setAlignment(); // center

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
		setTimeText("Time : " + Time2DisplayableString(winTime) + " min");
	}

	AudioController::getInstance()->flush();
	if (winGrade == TimeTargets::NR_OF_TIME_TARGETS)
		AudioController::getInstance()->play("lose", AudioController::SoundType::Effect);
	else
		AudioController::getInstance()->play("win1", AudioController::SoundType::Effect);
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();
	m_bowl.rotateY(dt * 0.5f);
	m_bowlContent.rotateY(dt * 0.5f);
	m_particleSystem.update(dt);

	if (m_btn_play.update_behavior(dt)) {
		SceneManager::getScene()->reset();
		pop(true);
	}
	if (m_btn_back.update_behavior(dt)) {
		AudioController::getInstance()->flush();
		pop(State::MainState, false);
	}
}

void EndRoundState::pause() { }

void EndRoundState::play() {
	float width = 1280;
	float height = 720;

	// Set the correct bowl
	// Bowl material and content are set in Playstate handleEvent(hasWon)
	m_bowl.setPosition(float3(0.0f, 0.25f, 0.0f));
	m_bowlContent.setPosition(m_bowl.getPosition());
	m_camera.setEye(float3(0.f, 0.9f, 1.0f));
	m_camera.setTarget(float3(0.f,0.3f,0.f));

	m_particleSystem.setPosition(m_bowl.getPosition() + float3(0.0f, 0, 0.f));

	m_btn_play.set(float2(float2(width/2 - 125, height - 100)), "Try Again", 0.2);
	m_btn_back.set(float2(float2(width / 2 + 125, height - 100)), "Level Select", 0.4);
	m_btn_back.setTextScale(0.7);
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

	m_spr_background.draw();
	m_bowl.draw();
	m_bowlContent.draw();
	m_particleSystem.draw();
	Renderer::getInstance()->clearDepth();

	float width = 1280;
	float height = 720;
	m_camera.bind();
	m_textRenderer.setAlignment(); // center
	m_textRenderer.setColor(Color(1., 1.f, 1.f, 1.0f));
	m_textRenderer.setPosition(float2(width / 2, height / 2 + 150));
	m_textRenderer.setText(m_timeText);
	m_textRenderer.draw();
	m_textRenderer.setColor(m_victoryColor);
	m_textRenderer.setPosition(float2(width / 2, 75));
	m_textRenderer.setText(m_victoryText);
	m_textRenderer.draw();

	m_btn_play.draw();
	m_btn_back.draw();

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
	float scale = 0.6;
	m_bowlContent.setScale(scale);

	m_bowl.load("Bowl");
	vector<string> bowlNames = { "bowl_gold", "bowl_silver", "bowl_bronze" };
	m_bowl.load(bowlNames[(int)bowlMaterial]);
	m_bowl.setScale(scale);
}

void EndRoundState::setTimeText(string text) { m_timeText = text; }

void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }

void EndRoundState::setVictoryText(string text) { m_victoryText = text; }

void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
