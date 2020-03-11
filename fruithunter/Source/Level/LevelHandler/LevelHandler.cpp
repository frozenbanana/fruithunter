#include "LevelHandler.h"
#include "TerrainManager.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "VariableSyncer.h"
#include "PathFindingThread.h"

void LevelHandler::initialiseLevel0() {
	Level level0;

	level0.m_terrainPropsFilename = "level0";

	level0.m_terrainTags.push_back(Level::TerrainTags::Volcano);
	level0.m_terrainTags.push_back(Level::TerrainTags::Forest);
	level0.m_terrainTags.push_back(Level::TerrainTags::Desert);
	level0.m_terrainTags.push_back(Level::TerrainTags::Plains);

	level0.m_fruitPos[APPLE].push_back(1);
	level0.m_fruitPos[APPLE].push_back(2);
	level0.m_fruitPos[APPLE].push_back(3);
	level0.m_fruitPos[BANANA].push_back(1);
	level0.m_fruitPos[BANANA].push_back(2);
	level0.m_fruitPos[BANANA].push_back(3);
	level0.m_fruitPos[MELON].push_back(0);

	level0.m_heightMapNames.push_back("VolcanoMap.png");
	level0.m_heightMapNames.push_back("ForestMap.png");
	level0.m_heightMapNames.push_back("DesertMap.png");
	level0.m_heightMapNames.push_back("PlainMap.png");

	level0.m_heightMapPos.push_back(float3(100.f, 0.f, 100.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 100.f));
	level0.m_heightMapPos.push_back(float3(100.f, 0.f, 0.f));

	level0.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level0.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level0.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level0.m_heightMapSubSize.push_back(XMINT2(15, 15));

	level0.m_heightMapDivision.push_back(XMINT2(16, 16));
	level0.m_heightMapDivision.push_back(XMINT2(16, 16));
	level0.m_heightMapDivision.push_back(XMINT2(16, 16));
	level0.m_heightMapDivision.push_back(XMINT2(16, 16));

	level0.m_heightMapScales.push_back(float3(1.f, 0.40f, 1.f) * 100);
	level0.m_heightMapScales.push_back(float3(1.f, 0.15f, 1.f) * 100);
	level0.m_heightMapScales.push_back(float3(1.f, 0.25f, 1.f) * 100);
	level0.m_heightMapScales.push_back(float3(1.f, 0.10f, 1.f) * 100);

	vector<string> maps(4);
	maps[0] = "texture_rock8.jpg"; // flat
	maps[1] = "texture_lava1.jpg"; // low flat
	maps[2] = "texture_rock2.jpg"; // tilt
	maps[3] = "texture_rock2.jpg"; // low tilt
	level0.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass3.jpg";
	maps[1] = "texture_sand1.jpg";
	maps[2] = "texture_mossyRock.jpg";
	maps[3] = "texture_mossyRock.jpg";
	level0.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_sand3.jpg";
	maps[1] = "texture_sand1.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level0.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass.jpg";
	maps[1] = "texture_rock4.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level0.m_heightmapTextures.push_back(maps);

	level0.m_wind.push_back(float3(0.f, 8.f, 0.f)); // Volcano
	level0.m_wind.push_back(float3(3.f, 0.f, 6.f)); // Forest
	level0.m_wind.push_back(float3(1.f, 0.f, 2.f)); // Desert
	level0.m_wind.push_back(float3(0.f, 0.f, 1.f)); // Plains

	level0.m_nrOfFruits[APPLE] = 20;
	level0.m_nrOfFruits[BANANA] = 15;
	level0.m_nrOfFruits[MELON] = 9;

	level0.m_winCondition[APPLE] = 2;
	level0.m_winCondition[BANANA] = 2;
	level0.m_winCondition[MELON] = 2;

	level0.m_playerStartPos = float3(20.f, 0.0f, 20.f);

	level0.m_timeTargets[GOLD] = 120;
	level0.m_timeTargets[SILVER] = 160;
	level0.m_timeTargets[BRONZE] = 200;

	m_levelsArr.push_back(level0);
	m_hud.setTimeTargets(level0.m_timeTargets);
}

void LevelHandler::placeBridge(float3 pos, float3 rot, float3 scale) {
	// Place floor / planks
	shared_ptr<Entity> newEntity = make_shared<Entity>();
	newEntity->load("RopeBridgeFloor");
	newEntity->setCollisionDataOBB();
	newEntity->setScale(scale);
	newEntity->setPosition(pos);
	newEntity->rotate(rot);
	m_collidableEntities.push_back(newEntity);

	// place railings

	newEntity = make_shared<Entity>();
	newEntity->load("RopeBridgeRailing1");
	newEntity->setCollisionDataOBB();
	newEntity->setScale(scale);
	newEntity->setPosition(pos);
	newEntity->rotate(rot);
	m_collidableEntities.push_back(newEntity);

	newEntity = make_shared<Entity>();
	newEntity->load("RopeBridgeRailing2");
	newEntity->setCollisionDataOBB();
	newEntity->setScale(scale);
	newEntity->setPosition(pos);
	newEntity->rotate(rot);
	m_collidableEntities.push_back(newEntity);
}

void LevelHandler::placeAllBridges() {
	placeBridge(float3(103.2f, 3.1f, 39.f), float3(0.f, -0.1f, -0.07f), float3(1.9f, 1.f, 1.4f));
	placeBridge(float3(35.f, 3.5f, 98.5f), float3(0.f, 1.7f, 0.02f), float3(1.6f, 1.f, 1.4f));
	placeBridge(float3(99.2f, 7.9f, 155.f), float3(0.f, 0.f, -0.09f), float3(1.3f, 1.f, 1.4f));
}

void LevelHandler::placeAllAnimals() {
	shared_ptr<Animal> animal = make_shared<Animal>("Gorilla", 10.f, 7.f, BANANA, 2, 10.f,
		float3(96.2f, 3.45f, 38.f), float3(90.2f, 3.7f, 49.f), XM_PI * 0.5f);
	m_Animals.push_back(animal);

	animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f, float3(37.f, 3.2f, 93.f),
		float3(20.f, 3.7f, 90.f), 0.f);
	m_Animals.push_back(animal);

	animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f, float3(90.f, 8.2f, 152.f),
		float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
	m_Animals.push_back(animal);
}

LevelHandler::LevelHandler() { initialise(); }

LevelHandler::~LevelHandler() { PathFindingThread::getInstance()->exitThread(); }

void LevelHandler::initialise() {

	m_player.initialize();
	m_terrainManager = TerrainManager::getInstance();
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

	initialiseLevel0();

	m_particleSystems.resize(5);
	m_particleSystems[0] = ParticleSystem(ParticleSystem::VULCANO_FIRE);
	m_particleSystems[0].setPosition(float3(150.f, 15.f, 150.f));
	m_particleSystems[1] = ParticleSystem(ParticleSystem::VULCANO_SMOKE);
	m_particleSystems[1].setPosition(float3(150.f, 29.f, 150.f));
	m_particleSystems[2] = ParticleSystem(ParticleSystem::GROUND_DUST);
	m_particleSystems[2].setPosition(float3(42.f, 4.f, 125.f));
	m_particleSystems[3] = ParticleSystem(ParticleSystem::FOREST_BUBBLE);
	m_particleSystems[3].setPosition(float3(50.f, 5.f, 40.f));
	m_particleSystems[4] = ParticleSystem(ParticleSystem::LAVA_BUBBLE);
	m_particleSystems[4].setPosition(float3(150.f, 0.f, 149.f));

	waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(50, 50), XMINT2(8, 8),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));
	float3 lavaSize(82.f, 0.f, 82.f);
	float3 lavaPos(150, 1.5f, 150);
	lavaEffect.initilize(SeaEffect::SeaEffectTypes::lava, XMINT2(20, 20), XMINT2(4, 4),
		lavaPos - lavaSize / 2.f, lavaSize + float3(0, 2.f, 0));
}

void LevelHandler::loadLevel(int levelNr) {
	if (m_currentLevel != levelNr) {
		Renderer::getInstance()->drawLoading();

		m_currentLevel = levelNr;
		Level currentLevel = m_levelsArr.at(levelNr);

		m_terrainProps.load(currentLevel.m_terrainPropsFilename);

		for (int i = 0; i < m_levelsArr.at(levelNr).m_heightMapNames.size(); i++) {
			m_terrainManager->add(currentLevel.m_heightMapPos.at(i),
				currentLevel.m_heightMapScales[i], currentLevel.m_heightMapNames.at(i),
				currentLevel.m_heightmapTextures[i], currentLevel.m_heightMapSubSize.at(i),
				currentLevel.m_heightMapDivision.at(i), currentLevel.m_wind.at(i));
		}

		for (int i = 0; i < currentLevel.m_nrOfFruits[APPLE]; i++) {
			int terrainTagNr = i % currentLevel.m_fruitPos[APPLE].size();
			shared_ptr<Apple> apple = make_shared<Apple>(
				m_terrainManager->getSpawnpoint(currentLevel.m_fruitPos[APPLE].at(terrainTagNr)));
			m_fruits.push_back(apple);
		}
		for (int i = 0; i < currentLevel.m_nrOfFruits[BANANA]; i++) {
			int terrainTagNr = i % currentLevel.m_fruitPos[BANANA].size();
			shared_ptr<Banana> banana = make_shared<Banana>(
				m_terrainManager->getSpawnpoint(currentLevel.m_fruitPos[BANANA].at(terrainTagNr)));
			m_fruits.push_back(banana);
		}
		for (int i = 0; i < currentLevel.m_nrOfFruits[MELON]; i++) {
			int terrainTagNr = i % currentLevel.m_fruitPos[MELON].size();
			shared_ptr<Melon> melon = make_shared<Melon>(
				m_terrainManager->getSpawnpoint(currentLevel.m_fruitPos[MELON].at(terrainTagNr)));
			m_fruits.push_back(melon);
		}

		m_player.setPosition(currentLevel.m_playerStartPos);

		m_currentTerrain = currentLevel.m_terrainTags[m_terrainManager->getTerrainIndexFromPosition(
			currentLevel.m_playerStartPos)];

		// temp
		float height =
			TerrainManager::getInstance()->getHeightFromPosition(currentLevel.m_playerStartPos);
		shared_ptr<Entity> newEntity = make_shared<Entity>();
		newEntity->load("bush1");
		newEntity->setScale(1);
		newEntity->setPosition(currentLevel.m_playerStartPos + float3(1.f, height, 1.f));
		newEntity->setCollisionDataOBB();
		m_collidableEntities.push_back(newEntity);
		newEntity = make_shared<Entity>();
		newEntity->load("Cactus_tall");
		newEntity->setScale(1);
		newEntity->setPosition(currentLevel.m_playerStartPos + float3(1.f, height, 3.f));
		newEntity->setCollisionDataOBB();
		m_collidableEntities.push_back(newEntity);

		placeAllBridges();
		placeAllAnimals();

		m_hud.setTimeTargets(currentLevel.m_timeTargets);
		m_hud.setWinCondition(currentLevel.m_winCondition);


		if (currentLevel.m_nrOfFruits[APPLE] != 0)
			m_hud.createFruitSprite("apple");
		if (currentLevel.m_nrOfFruits[BANANA] != 0)
			m_hud.createFruitSprite("banana");
		if (currentLevel.m_nrOfFruits[MELON] != 0)
			m_hud.createFruitSprite("melon");
	}

	if (PathFindingThread::getInstance()->m_thread == nullptr) {
		PathFindingThread::getInstance()->initialize(m_fruits, m_frame, m_collidableEntities);
	}
}

void LevelHandler::draw() {
	m_skyBox.bindLightBuffer();
	m_player.draw();
	Renderer::getInstance()->enableAlphaBlending();
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate();
	}
	Renderer::getInstance()->disableAlphaBlending();

	for (size_t i = 0; i < m_Animals.size(); ++i) {
		m_Animals[i]->draw();
	}

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->draw();
	}
	m_entity.draw();
	m_skyBox.draw(m_oldTerrain, m_currentTerrain);

	// frustum data for culling
	vector<FrustumPlane> frustum = m_player.getFrustumPlanes();
	// terrain entities
	m_terrainProps.draw_quadtreeFrustumCulling(frustum);

	// terrain
	m_terrainManager->draw_quadtreeFrustumCulling(frustum);
	// water/lava effect
	Renderer::getInstance()->copyDepthToSRV();
	waterEffect.draw_quadtreeFrustumCulling(frustum);
	lavaEffect.draw_quadtreeFrustumCulling(frustum);

	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */
	m_hud.draw(); // TODO: Find out why hud is not drawn if particleSystems are before

	// Particle Systems
	for (size_t i = 0; i < m_particleSystems.size(); i++) {
		m_particleSystems[i].draw();
	}

	m_player.getBow().getTrailEffect().draw();
}

void LevelHandler::drawShadowDynamic() {
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate_shadow();
	}
	m_terrainManager->drawShadow();

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->drawShadow();
	}
	m_terrainProps.drawShadow();
}

void LevelHandler::drawShadowStatic() {
	m_terrainManager->drawShadow();

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->drawShadow();
	}

	m_terrainProps.drawShadow();
}

void LevelHandler::drawShadowDynamicEntities() {
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate_shadow();
	}
}

void LevelHandler::update(float dt) {
	auto pft = PathFindingThread::getInstance();

	m_terrainProps.update(m_player.getCameraPosition(), m_player.getForward());

	m_skyBox.updateDelta(dt);

	if (Input::getInstance()->keyPressed(Keyboard::R) && m_currentLevel >= 0)
		m_player.setPosition(m_levelsArr[m_currentLevel].m_playerStartPos);

	m_player.update(dt, m_terrainManager->getTerrainFromPosition(m_player.getPosition()));
	m_player.getBow().getTrailEffect().update(dt);

	if (m_player.inHuntermode()) {
		dt *= 0.1f;
	}

	// for all animals
	for (size_t i = 0; i < m_Animals.size(); ++i) {
		if (m_Animals[i]->notBribed()) {
			bool getsThrown = m_player.checkAnimal(m_Animals[i]->getPosition(),
				m_Animals[i]->getPlayerRange(), m_Animals[i]->getThrowStrength());
			if (getsThrown) {
				m_Animals[i]->makeAngrySound();
				m_Animals[i]->beginWalk(m_player.getPosition());
			}
			else {
				m_Animals[i]->setAttacked(false);
			}


			for (size_t iFruit = 0; iFruit < m_fruits.size(); ++iFruit) {
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
			}
		}
		m_Animals[i]->update(dt, m_player.getPosition());
	}

	dropFruit();

	float3 playerPos = m_player.getPosition();

	// update terrain tag
	int activeTerrain = m_terrainManager->getTerrainIndexFromPosition(playerPos);

	if (activeTerrain == 2) {
		AudioHandler::getInstance()->changeMusicTo(AudioHandler::SPANISH_GUITAR, dt);
	}
	else if (activeTerrain == 1) {
		AudioHandler::getInstance()->changeMusicTo(AudioHandler::KETAPOP, dt);
	}
	else if (activeTerrain == 0) {
		AudioHandler::getInstance()->changeMusicTo(AudioHandler::KETAPOP_DARK, dt);
	}
	else {
		AudioHandler::getInstance()->changeMusicTo(AudioHandler::JINGLE_GUITAR, dt);
	}

	if (activeTerrain != -1 && m_currentLevel != -1) {
		Level::TerrainTags tag = m_levelsArr[m_currentLevel].m_terrainTags[activeTerrain];
		if (m_currentTerrain != tag) {
			m_oldTerrain = m_currentTerrain;
			m_currentTerrain = tag;
			m_skyBox.resetDelta();
			m_skyBox.updateNewOldLight(tag);
		}
	}

	m_skyBox.updateCurrentLight();

	// update stuff
	for (int i = 0; i < m_fruits.size(); i++) {
		pft->m_mutex.lock();
		m_fruits[i]->update(dt, playerPos);
		if (m_player.isShooting()) {

			if (m_player.getArrow().checkCollision(*m_fruits[i])) {
				m_fruits[i]->hit();
				AudioHandler::getInstance()->playOnceByDistance(
					AudioHandler::HIT_FRUIT, m_player.getPosition(), m_fruits[i]->getPosition());

				m_player.getArrow().setPosition(
					float3(-100.f)); // temporary to disable arrow until returning
			}
		}
		if (float3(m_fruits[i].get()->getPosition() - m_player.getPosition()).Length() <
			1.5f) { // If the fruit is close to the player get picked up
			pickUpFruit(m_fruits[i].get()->getFruitType());
			AudioHandler::getInstance()->playOnce(AudioHandler::COLLECT);
			m_fruits.erase(m_fruits.begin() + i);
		}
		pft->m_mutex.unlock();
	}

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_player.collideObject(*m_collidableEntities[i]);
	}

	// Check entity collisions
	// player - entity
	vector<unique_ptr<Entity>>* entities = m_terrainProps.getEntities();
	for (size_t iObj = 0; iObj < entities->size(); ++iObj) {
		m_player.collideObject(*entities->at(iObj));
	}



	// Check entity - arrow
	float3 arrowPosision = m_player.getArrow().getPosition();
	float3 arrowVelocity = m_player.getBow().getArrowVelocity();
	vector<Entity**> entitiesAroundArrow =
		m_terrainProps.getCulledEntitiesByPosition(arrowPosision);
	if (m_player.isShooting() && !m_player.getBow().getArrowHitObject()) {
		for (size_t i = 0; i < entitiesAroundArrow.size(); i++) {
			if ((*entitiesAroundArrow[i])->getIsCollidable()) {
				float castray =
					(*entitiesAroundArrow[i])->castRay(arrowPosision, arrowVelocity * dt);
				if (castray != -1.f && castray < 1.f) {
					// Arrow is hitting object
					float3 target = arrowPosision + arrowVelocity * dt * castray;
					m_player.getBow().arrowHitObject(target);
				}
			}
		}
	}



	for (size_t i = 0; i < m_particleSystems.size(); i++) {
		Terrain* currentTerrain =
			m_terrainManager->getTerrainFromPosition(m_particleSystems[i].getPosition());
		m_particleSystems[i].update(dt, currentTerrain->getWind());
	}

	m_hud.update(dt, m_player.getStamina());
	waterEffect.update(dt);
	lavaEffect.update(dt);

	// Renderer::getInstance()->setPlayerPos(playerPos);
}

HUD& LevelHandler::getHUD() { return m_hud; }

void LevelHandler::pickUpFruit(int fruitType) {
	m_inventory[fruitType]++;
	m_hud.addFruit(fruitType);
}

void LevelHandler::dropFruit() {
	Input* ip = Input::getInstance();
	auto pft = PathFindingThread::getInstance();

	if (ip->keyPressed(Keyboard::D1)) {
		if (m_inventory[APPLE] >= 0) {
			shared_ptr<Apple> apple =
				make_shared<Apple>(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f));
			apple->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(apple);
			pft->m_mutex.unlock();

			// m_inventory[APPLE]--;
			m_hud.removeFruit(APPLE);
		}
	}
	if (ip->keyPressed(Keyboard::D2)) {
		if (m_inventory[BANANA] >= 0) {
			shared_ptr<Banana> banana =
				make_shared<Banana>(float3(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
			banana->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(banana);
			pft->m_mutex.unlock();
			// m_inventory[BANANA]--;
			m_hud.removeFruit(BANANA);
		}
	}
	if (ip->keyPressed(Keyboard::D3)) {
		if (m_inventory[MELON] >= 0) {
			shared_ptr<Melon> melon =
				make_shared<Melon>((m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
			melon->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(melon);
			pft->m_mutex.unlock();
			// m_inventory[MELON]--;
			m_hud.removeFruit(MELON);
		}
	}
}

float3 LevelHandler::getPlayerPos() { return m_player.getPosition(); }
