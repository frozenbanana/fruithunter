#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "TerrainManager.h"
#include "Settings.h"

#include "WICTextureLoader.h"



IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	m_startButton.initialize("Start", float2(106, height * 0.75f - 60.f));
	m_settingsButton.initialize("Settings", float2(132, height * 0.75f));
	m_exitButton.initialize("Exit", float2(92, height * 0.75f + 60.f));

	// Initialise camera
	// m_camera.setView(float3(61.4f, 16.8f, 44.4f), float3(61.2f, 7.16f, 28.7f), float3(0.f, 1.f,
	// 0.f));
	m_camera.setView(
		// float3(56.4f, 11.0f, 18.2f), float3(68.9f, 10.64f, 23.9f), float3(0.f, 1.f, 0.f));
		float3(58.0f, 10.9f, 21.9f), float3(61.3f, 10.1f, -36.0f), float3(0.f, 1.f, 0.f));

	// Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

	// Initialise Terrain
	m_maps.push_back("texture_grass.jpg");
	m_maps.push_back("texture_rock6.jpg");
	m_maps.push_back("texture_rock4.jpg");
	m_maps.push_back("texture_rock6.jpg");

	// Initiate terrainProps
	m_terrainProps.load("menu");

	// Initiate apple
	m_apple = make_unique<Apple>(float3(58.0f, 10.1f, 16.9f));
	// m_apple.setPosition(float3(58.f, 11.0f, 18.0f));

	// m_letterPaths[0] = L"assets/sprites/fruithunter_logo2.png";

	m_letters.resize(11);
	string logoPaths[11] = {
		"fruithunter_logo_F_color.png",
		"fruithunter_logo_r_color.png",
		"fruithunter_logo_u_color.png",
		"fruithunter_logo_i_color.png",
		"fruithunter_logo_t_color.png",
		"fruithunter_logo_H_color.png",
		"fruithunter_logo_u_color.png",
		"fruithunter_logo_n_color.png",
		"fruithunter_logo_t_color.png",
		"fruithunter_logo_e_color.png",
		"fruithunter_logo_r_color.png",
	};
	for (size_t i = 0; i < m_letters.size(); i++) {
		m_letters[i].letter.load(logoPaths[i]);
		m_letters[i].letter.setScale(0.25f);
		m_letters[i].speedOffset = float2(RandomFloat(-0.15f, 0.15f), RandomFloat(-0.5f, 0.5f));
		m_letters[i].letter.setAlignment();//center
	}

	m_timer.reset();
}

void IntroState::update() {
	float3 bowPos(68.9f, 10.64f, 23.9f);
	float3 treePos(56.4f, 9.0f, 18.2f);

	m_timer.update();
	float delta = m_timer.getDt();
	m_totalDelta = fmod((m_totalDelta + delta), (2.f * XM_PI));
	m_shootTime += delta;

	Input::getInstance()->setMouseModeAbsolute();
	m_skybox.updateNewOldLight(2);
	m_skybox.updateCurrentLight();
	m_waterEffect.update(delta);

	m_apple.get()->setPosition(float3(
		treePos.x + (cos(m_totalDelta) * 2.0f), treePos.y, treePos.z + (sin(m_totalDelta) * 2.0f)));

	m_apple.get()->update(delta, m_camera.getPosition());
	m_apple.get()->setRotation(float3(0.0f, -m_totalDelta, 0.0f));

	// float3 bowForward(56.4f - 68.9f, 9.0f - 9.64f, 18.2f - 23.9f);
	float3 bowForward = treePos - bowPos;
	bowForward.Normalize();
	if (m_shootTime > m_chargeThreshold) {
		m_bow.charge();
	}
	m_bow.update(delta, bowPos, bowForward, bowForward.Cross(float3(0.f, 1.0f, 0.f)),
		TerrainManager::getInstance()->getTerrainFromPosition(bowPos));

	if (m_shootTime > m_shootThreshold) {
		m_shootTime = 0.f;
		m_bow.shoot(float3(bowForward.x, bowForward.y + RandomFloat(0.12f, 0.2f),
			bowForward.z + RandomFloat(0.02f, 0.06f)),
			float3(0.f), -0.003f, 4.224f);
	}
	m_bow.getTrailEffect().update(
		delta, TerrainManager::getInstance()->getTerrainFromPosition(bowPos));
	m_timer.update();

	// Arrow collision
	if (!m_bow.getArrowHitObject()) {
		for (int i = 0; i < m_terrainProps.getEntities()->size(); i++) {
			float rayCastingValue = m_terrainProps.getEntities()->at(i)->castRay(
				m_bow.getArrow().getPosition(), m_bow.getArrowVelocity() * delta);
			if (rayCastingValue != -1.f && rayCastingValue < 1.f) {
				// Arrow is hitting object
				float3 target = m_bow.getArrow().getPosition() +
					m_bow.getArrowVelocity() * delta * rayCastingValue;
				m_bow.arrowHitObject(target);
				m_chargeThreshold = RandomFloat(2.4f, 4.f);
				m_shootDelay = RandomFloat(0.2f, 1.f);
				m_shootThreshold = m_chargeThreshold + m_shootDelay;
				shared_ptr<Entity> newArrow = make_shared<Entity>(m_bow.getArrow().getModelName(),
					m_bow.getArrow().getPosition(), m_bow.getArrow().getScale());
				newArrow->setRotationMatrix(m_bow.getArrow().getRotationMatrix());
				m_arrows.push_back(newArrow);
			}
		}
	}

	// Logo update
	float offsetX = 1280.f / 16.f;
	float offsetY = 720.f / 3.f;
	float t = m_timer.getTimePassed();
	for (size_t i = 0; i < m_letters.size(); i++) {
		float2 movement =
			float2(sin(t + m_letters[i].speedOffset.x), cos(t + m_letters[i].speedOffset.y)) * 10.f;
		m_letters[i].letter.setPosition(float2(offsetX, offsetY) + movement);
		offsetX += m_letters[i].letter.getTextureSize().x / (1.65f * 2.f);
	}

	Input::getInstance()->setMouseModeAbsolute();
}

void IntroState::handleEvent() {
	if (m_startButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void IntroState::draw() {
	// Draw to shadowmap
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_camera.getFrustumPoints(0.1f));
	shadowMap->setup_depthRendering();

	Renderer::getInstance()->enableAlphaBlending();
	m_apple.get()->draw_animate_onlyMesh();
	Renderer::getInstance()->disableAlphaBlending();
	m_bow.draw();
	m_bow.getTrailEffect().draw();
	for (int i = 0; i < m_arrows.size(); i++) {
		m_arrows.at(i)->draw_onlyMesh(float3(1.f));
	}
	m_terrainProps.draw_onlyMesh();
	TerrainManager::getInstance()->draw_onlyMesh();

	// Scene drawing
	Renderer::getInstance()->beginFrame();
	shadowMap->setup_shadowsRendering();
	m_skybox.bindLightBuffer();
	m_camera.bindMatrix();

	m_bow.draw();
	m_terrainProps.draw();
	for (int i = 0; i < m_arrows.size(); i++) {
		m_arrows.at(i)->draw();
	}
	TerrainManager::getInstance()->draw();
	Renderer::getInstance()->copyDepthToSRV();
	m_waterEffect.draw();

	Renderer::getInstance()->enableAlphaBlending();
	m_apple.get()->draw_animate();
	Renderer::getInstance()->disableAlphaBlending();

	Renderer::getInstance()->draw_darkEdges();
	m_bow.getTrailEffect().draw();
	m_skybox.draw(2, 2);

	// Logo
	for (size_t i = 0; i < m_letters.size(); i++)
		m_letters[i].letter.draw();

	// Draw menu buttons
	m_startButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();
}


void IntroState::play() {
	ErrorLogger::log(m_name + " play() called.");

	Settings::getInstance()->loadAllSetting();

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	AudioHandler::getInstance()->playMusic(AudioHandler::Music::OCEAN);

	m_startButton.setPosition(float2(106, height * 0.75f - 60.f));
	m_settingsButton.setPosition(float2(132, height * 0.75f));
	m_exitButton.setPosition(float2(92, height * 0.75f + 60.f));

	TerrainManager::getInstance()->removeAll();
	TerrainManager::getInstance()->add(float3(0.f), float3(1.f, 0.10f, 1.f) * 100, "PlainMap.png",
		m_maps, XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));
}
