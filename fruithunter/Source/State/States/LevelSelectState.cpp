#include "LevelSelectState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Statehandler.h"
#include "PlayState.h"
#include "SaveManager.h"

void LevelSelectState::initialize() {
	m_name = "Level select state";

	// Initiate player
	m_player.setPosition(m_spawnPosition);
	// Initiate terrain
	// m_maps = vector<string> maps(4);
	m_maps.push_back("texture_grass.jpg");
	m_maps.push_back("texture_rock6.jpg");
	m_maps.push_back("texture_rock4.jpg");
	m_maps.push_back("texture_rock6.jpg");

	// Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

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
	float dt = m_timer.getDt();

	// Update terrainprops
	m_terrainProps.update(dt, m_player.getCameraPosition(), m_player.getForward());

	// update player
	m_player.update(dt);

	for (int i = 0; i < m_terrainProps.getEntities()->size(); i++) {
		m_player.collideObject(*m_terrainProps.getEntities()->at(i));
	}

	// Update Skybox
	m_skyBox.update(dt);
	m_skyBox.switchLight(AreaTag::Plains);

	// update water
	m_waterEffect.update(dt);

	// Update bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		// if index inside array and previous level is completed, then allow to play level
		if (i == 0 || SaveManager::getInstance()->getActiveSave()[i - 1].isCompleted) {
			// Check collision
			if (m_player.getArrow().checkCollision(m_levelSelectors[i].m_bowl)) {
				m_player.getArrow().setPosition(float3(-1000.f));
				m_player.setPosition(m_spawnPosition);
				TerrainManager::getInstance()->removeAll();
				draw(); // Updates hitboxes and prepares state for next time.
				setLevel(i);
				StateHandler::getInstance()->changeState(StateHandler::PLAY);
			}
		}
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
		}
		m_animal[i]->update(dt, m_player.getPosition());
	}

	// Check entity - arrow
	vector<Entity**> entitiesAroundArrow =
		m_terrainProps.getCulledEntitiesByPosition(m_player.getArrow().getPosition());
	for (size_t i = 0; i < entitiesAroundArrow.size(); i++) {
		if ((*entitiesAroundArrow[i])->getIsCollidable()) {
			m_player.arrowCollideToEntity(**entitiesAroundArrow[i], dt);
		}
	}

}

void LevelSelectState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}
}

void LevelSelectState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void LevelSelectState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	AudioHandler::getInstance()->changeMusicTo(AudioHandler::ELEVATOR, 0.f); // dt not used. Lazy...
	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->destroyLevel(); // reset if there is an old level
	TerrainManager::getInstance()->removeAll();
	TerrainManager::getInstance()->add(float3(0.f), float3(100.f, 25.f, 100.f), "tutorial.png",
		m_maps, XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));

	initializeLevelSelectors();
}

void LevelSelectState::draw() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_player.getFrustumPoints(0.4f));
	shadowMap->setup_depthRendering();

	for (int i = 0; i < m_levelSelectors.size(); i++) {
		m_levelSelectors[i].m_bowl.draw_onlyMesh(float3(0, 0, 0));
		m_levelSelectors[i].m_content.draw_onlyMesh(float3(0, 0, 0));
	}
	// draw terrain
	TerrainManager::getInstance()->draw_onlyMesh();
	// draw terrain props
	m_terrainProps.draw_onlyMesh();
	// draw animals
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw_onlyAnimal();
	}

	// Set first person info
	Renderer::getInstance()->beginFrame();
	shadowMap->setup_shadowRendering();

	// draw first person
	m_skyBox.bindLightBuffer();
	m_player.bindMatrix();
	//draw bow
	m_player.draw();
	// draw bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		m_levelSelectors[i].m_bowl.draw();
		m_levelSelectors[i].m_content.draw();
	}
	// draw terrain entities
	m_terrainProps.draw();
	// draw animals
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw_onlyAnimal();
	}
	TerrainManager::getInstance()->draw();
	Renderer::getInstance()->copyDepthToSRV();
	m_waterEffect.draw();

	// text above bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		LevelData levelData = SaveManager::getInstance()->getActiveSave()[i];
		size_t minutes = levelData.timeOfCompletion / 60;
		size_t seconds = levelData.timeOfCompletion % 60;
		string strMinutes = "--";
		string strSeconds = "--";
		if (SaveManager::getInstance()->getActiveSave()[i].isCompleted) {
			strMinutes = (minutes < 10 ? "0" : "") + to_string(minutes);
			strSeconds = (seconds < 10 ? "0" : "") + to_string(seconds);
		}
		string str = (levelData.isCompleted ? "COMPLETED" : "");
		str += "\nBest Time: " + strMinutes + "." + strSeconds + " Minutes";
		m_textRenderer.drawTextInWorld(str,
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0),
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0) - m_player.getForward(),
			float2(1.f) * 4.f);
	}
	// skybox
	m_skyBox.draw();
	// dark edges
	Renderer::getInstance()->draw_darkEdges();
	m_player.getBow().getTrailEffect().drawNoAlpha();
}

LevelSelectState::~LevelSelectState() {}

void LevelSelectState::setLevel(int newLevel) {

	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->setLevel(newLevel);
}

void LevelSelectState::initializeLevelSelectors() {
	// initiate level selectors
	float3 bowlPositions[3]{ float3(7.3f, 2.7f, 47.4f), float3(41.7f, 2.75f, 20.6f),
		float3(90.6f, 2.7f, 47.0f) };
	vector<string> bowlMaterials;
	bowlMaterials.resize(TimeTargets::NR_OF_TIME_TARGETS + 1);
	bowlMaterials[TimeTargets::GOLD] = "BowlGold.mtl";
	bowlMaterials[TimeTargets::SILVER] = "BowlSilver.mtl";
	bowlMaterials[TimeTargets::BRONZE] = "BowlBronze.mtl";
	bowlMaterials[TimeTargets::NR_OF_TIME_TARGETS] = "Bowl.mtl";
	string bowlContent[3] = { "BowlContent1", "BowlContent2", "BowlContent3" };
	m_levelSelectors.resize(3);
	for (size_t i = 0; i < 3; i++) {
		bool isCompleted = SaveManager::getInstance()->getActiveSave()[i].isCompleted;
		TimeTargets grade = SaveManager::getInstance()->getActiveSave()[i].grade;
		m_levelSelectors[i].m_bowl.load("Bowl");
		m_levelSelectors[i].m_bowl.setPosition(bowlPositions[i]);
		m_levelSelectors[i].m_bowl.loadMaterials(bowlMaterials);
		if (isCompleted) {
			m_levelSelectors[i].m_bowl.setCurrentMaterial(grade);
			m_levelSelectors[i].m_content.load(bowlContent[i]);
			m_levelSelectors[i].m_content.setPosition(bowlPositions[i]);
		}
		else {
			m_levelSelectors[i].m_bowl.setCurrentMaterial(
				TimeTargets::NR_OF_TIME_TARGETS); // use bowl.mtl material, ignore strange index
		}
	}

	// Initiate animals
	m_animal.clear();
	shared_ptr<Animal> animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f,
		float3(41.369f, 2.746f, 50.425f), float3(20.f, 3.7f, 90.f), 0.f);
	if (!SaveManager::getInstance()->getActiveSave()[0].isCompleted)
		m_animal.push_back(animal);

	animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f, float3(52.956f, 2.752f, 65.128f),
		float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
	if (!SaveManager::getInstance()->getActiveSave()[1].isCompleted)
		m_animal.push_back(animal);
}
