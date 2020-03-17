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
	// m_maps = vector<string> maps(4);
	m_maps.push_back("texture_grass.jpg");
	m_maps.push_back("texture_rock6.jpg");
	m_maps.push_back("texture_rock4.jpg");
	m_maps.push_back("texture_rock6.jpg");

	// Initiate animals
	shared_ptr<Animal> animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f,
		float3(41.369f, 2.746f, 50.425f), float3(20.f, 3.7f, 90.f), 0.f);
	m_animal.push_back(animal);

	animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f, float3(52.956f, 2.752f, 65.128f),
		float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
	m_animal.push_back(animal);

	// Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

	// Initiate sky box


	// Initiate fruit bowls
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		m_bowls[i] = new Entity("bowl", m_bowlPos[i], float3(1));
	}

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
	m_terrainProps.update(delta, m_player.getCameraPosition(), m_player.getForward());

	// update player
	m_player.update(
		delta, TerrainManager::getInstance()->getTerrainFromPosition(m_player.getPosition()));

	for (int i = 0; i < m_terrainProps.getEntities()->size(); i++) {
		m_player.collideObject(*m_terrainProps.getEntities()->at(i));
	}

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
			AudioHandler::getInstance()->pauseAllMusic();
			draw(); // Updates hitboxes and prepares state for next time.
			setLevel(i);
			StateHandler::getInstance()->changeState(StateHandler::PLAY);
		}

		m_bowls[i]->updateAnimated(delta);
	}

	// Update animals
	// for all animals
	for (size_t i = 0; i < m_animal.size(); ++i) {
		m_animal[i]->checkLookedAt(m_player.getCameraPosition(), m_player.getForward());
		if (m_animal[i]->notBribed()) {
			bool getsThrown = m_player.checkAnimal(m_animal[i]->getPosition(),
				m_animal[i]->getPlayerRange(), m_animal[i]->getThrowStrength());
			if (getsThrown) {
				m_animal[i]->makeAngrySound();
				m_animal[i]->beginWalk(m_player.getPosition());
			}
			else {
				m_animal[i]->setAttacked(false);
			}


			/*for (size_t iFruit = 0; iFruit < m_fruits.size(); ++iFruit) {
				pft->m_mutex.lock();
				if (m_fruits[iFruit]->getFruitType() == m_Animals[i]->getfruitType()) {
					float len =
						(m_Animals[i]->getPosition() - m_fruits[iFruit]->getPosition()).Length();
					if (len < m_Animals[i]->getFruitRange()) {
						m_Animals[i]->grabFruit(m_fruits[iFruit]->getPosition());
						m_fruits.erase(m_fruits.begin() + iFruit);
					}
				}
				pft->m_mutex.unlock();
			}*/
		}
		m_animal[i]->update(delta, m_player.getPosition());
	}
}

void LevelSelectState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}
}

void LevelSelectState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
}

void LevelSelectState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	AudioHandler::getInstance()->changeMusicTo(AudioHandler::ELEVATOR, 0.f); // dt not used. Lazy...
	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->destroyLevel(); // reset if there is an old level
	TerrainManager::getInstance()->removeAll();
	TerrainManager::getInstance()->add(float3(0.f), float3(100.f, 25.f, 100.f), "tutorial.png",
		m_maps, XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));
}

void LevelSelectState::draw() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_player.getFrustumPoints(0.4f));
	shadowMap->setup_depthRendering();

	for (int i = 0; i < NR_OF_LEVELS; i++) {
		m_bowls[i]->draw();
	}
	m_terrainProps.draw();
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw();
	}
	TerrainManager::getInstance()->draw();

	// Set first person info
	Renderer::getInstance()->beginFrame();
	shadowMap->setup_shadowsRendering();

	// draw first person
	m_skyBox.bindLightBuffer();
	m_player.draw();
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		m_bowls[i]->draw();
	}
	m_terrainProps.draw();
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw();
	}
	TerrainManager::getInstance()->draw();
	Renderer::getInstance()->copyDepthToSRV();
	m_waterEffect.draw();
	Renderer::getInstance()->draw_darkEdges();
	m_skyBox.draw(2, 2);
}

LevelSelectState::~LevelSelectState() {
	for (int i = 0; i < NR_OF_LEVELS; i++) {
		delete m_bowls[i];
	}
}

void LevelSelectState::setLevel(int newLevel) {

	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->setLevel(newLevel);
}