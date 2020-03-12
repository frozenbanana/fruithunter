#include "LevelSelectState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Statehandler.h"
#include "PlayState.h"

void LevelSelectState::initialize() {
	m_name = "Level select state";

	// Initiate player
	m_player.initialize();
	m_player.setPosition(float3(34.0f, 2.5f, 79.9f));

	// Initiate terrain
	vector<string> maps(4);
	maps[0] = "texture_grass.jpg";
	maps[1] = "texture_rock4.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";

	TerrainManager::getInstance()->removeAll();
	TerrainManager::getInstance()->add(float3(0.f), float3(100.f, 25.f, 100.f),"tutorial.png", maps,
		XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));
	// Initiate props


	// Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

	// Initiate sky box


	// Initiate fruit bowls
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		m_bowls[i] = new Entity("bowl", m_bowlPos[i], float3(1));
	}

	// Initate shadowmap
	m_shadowMap = make_unique<ShadowMapper>();

	// Initiate entity repos
	m_terrainProps.addPlaceableEntity("treeMedium1");
	m_terrainProps.addPlaceableEntity("treeMedium2");
	m_terrainProps.addPlaceableEntity("treeMedium3");
	m_terrainProps.addPlaceableEntity("stone1");
	m_terrainProps.addPlaceableEntity("stone2");
	m_terrainProps.addPlaceableEntity("stone3");
	m_terrainProps.addPlaceableEntity("bush1");
	m_terrainProps.addPlaceableEntity("bush2");
	m_terrainProps.addPlaceableEntity("DeadBush");
	m_terrainProps.addPlaceableEntity("BurnedTree1");
	m_terrainProps.addPlaceableEntity("BurnedTree2");
	m_terrainProps.addPlaceableEntity("BurnedTree3");
	m_terrainProps.addPlaceableEntity("Cactus_tall");
	m_terrainProps.addPlaceableEntity("Cactus_small");
	m_terrainProps.addPlaceableEntity("Grass1");
	m_terrainProps.addPlaceableEntity("Grass2");
	m_terrainProps.addPlaceableEntity("Grass3");
	m_terrainProps.addPlaceableEntity("Grass4");
	m_terrainProps.addPlaceableEntity("RopeBridgeFloor");
	m_terrainProps.addPlaceableEntity("RopeBridgeRailing1");
	m_terrainProps.addPlaceableEntity("RopeBridgeRailing2");

	m_terrainProps.load("levelSelection");
}

void LevelSelectState::update() {
	Input::getInstance()->setMouseModeRelative();

	m_timer.update();
	float delta = m_timer.getDt();

	// Update terrainprops
	m_terrainProps.update(m_player.getCameraPosition(), m_player.getForward());

	// update player
	m_player.update(delta, TerrainManager::getInstance()->getTerrainFromPosition(m_player.getPosition()));

	// ErrorLogger::logFloat3("playerPos: ", m_player.getPosition());

	// Update Skybox
	m_skyBox.updateDelta(delta);
	m_skyBox.updateCurrentLight();
	m_skyBox.updateNewOldLight(2);

	// update water
	m_waterEffect.update(delta);

	// Update bowls
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		// Check collision
		if (m_player.getArrow().checkCollision(*m_bowls[i])) {
			m_player.getArrow().setPosition(float3(-1000.f));
			m_player.setPosition(float3(34.0f, 2.5f, 79.9f));
			TerrainManager::getInstance()->removeAll();
			draw(); // Updates hitboxes and prepares state for next time.
			setLevel(i);
			StateHandler::getInstance()->changeState(StateHandler::PLAY);
		}

		m_bowls[i]->updateAnimated(delta);
	}
}

void LevelSelectState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}
}

void LevelSelectState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void LevelSelectState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	AudioHandler::getInstance()->changeMusicTo(AudioHandler::ELEVATOR, 0.f); // dt not used. Lazy...
	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->destroyLevel(); // reset if there is an old level
}

void LevelSelectState::draw() {
	if (1) {
		m_shadowMap.get()->update(m_player.getPosition()); // not needed?

		if (m_staticShadowNotDrawn) {
			//	Set static shadow map info
			m_shadowMap.get()->bindDSVAndSetNullRenderTargetStatic();
			m_shadowMap.get()->bindCameraMatrix();

			// Draw static shadow map
			// m_terrain->drawShadow();
			//m_terrain->draw();
			/*Draw collidables*/
			/*Draw terrainprops*/
			m_staticShadowNotDrawn = false;
		}
		// Set shadow map info
		m_shadowMap.get()->bindDSVAndSetNullRenderTarget();
		m_shadowMap.get()->bindCameraMatrix();

		// Draw shadow map
		for (int i = 0; i < NR_OF_LEVELS; i++) {
			m_bowls[i]->drawShadow();
		}
	}

	// Set first person info
	Renderer::getInstance()->beginFrame();
	m_shadowMap.get()->bindVPTMatrix();
	m_shadowMap.get()->bindShadowMap();

	// draw first person
	m_skyBox.bindLightBuffer();
	m_player.draw();
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		m_bowls[i]->draw();
	}
	m_terrainProps.draw();
	//m_terrain->draw();
	TerrainManager::getInstance()->draw();
	Renderer::getInstance()->copyDepthToSRV();
	m_waterEffect.draw();
	m_skyBox.draw(2, 2);
}

LevelSelectState::~LevelSelectState() {
	delete m_terrain;
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		delete m_bowls[i];
	}
}

void LevelSelectState::setLevel(int newLevel) {

	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->setLevel(newLevel);
}