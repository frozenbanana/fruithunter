#include "LevelHandler.h"
#include "TerrainManager.h"
#include "AudioHandler.h"
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

	level0.m_heightMapSubSize.push_back(XMINT2(250, 250));
	level0.m_heightMapSubSize.push_back(XMINT2(250, 250));
	level0.m_heightMapSubSize.push_back(XMINT2(250, 250));
	level0.m_heightMapSubSize.push_back(XMINT2(250, 250));

	level0.m_heightMapDivision.push_back(XMINT2(1, 1));
	level0.m_heightMapDivision.push_back(XMINT2(1, 1));
	level0.m_heightMapDivision.push_back(XMINT2(1, 1));
	level0.m_heightMapDivision.push_back(XMINT2(1, 1));

	level0.m_heightMapScales.push_back(float3(1.f, 0.20f, 1.f) * 100);
	level0.m_heightMapScales.push_back(float3(1.f, 0.15f, 1.f) * 100);
	level0.m_heightMapScales.push_back(float3(1.f, 0.20f, 1.f) * 100);
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

	level0.m_nrOfFruits[APPLE] = 0;
	level0.m_nrOfFruits[BANANA] = 0;
	level0.m_nrOfFruits[MELON] = 0;

	level0.m_playerStartPos = float3(20.f, 0.0f, 20.f);

	level0.m_timeTargets[GOLD] = 20;
	level0.m_timeTargets[SILVER] = 35;
	level0.m_timeTargets[BRONZE] = 80;

	m_levelsArr.push_back(level0);
	m_hud.setTimeTargets(level0.m_timeTargets);

	m_frame = make_unique<size_t>();
	/**m_frame = 0;*/
	// m_thread = make_unique<PathFindingThread>(m_fruits, m_frame, m_collidableEntities);
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
	placeBridge(float3(35.f, 3.2f, 99.f), float3(0.f, 1.7f, 0.13f), float3(1.6f, 1.f, 1.4f));
	placeBridge(float3(98.f, 8.2f, 152.f), float3(0.f, -0.1f, -0.13f), float3(1.8f, 1.f, 1.4f));
}

LevelHandler::LevelHandler() { initialise(); }

LevelHandler::~LevelHandler() {}

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
	m_terrainProps.addPlaceableEntity("Block");

	initialiseLevel0();

	waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));
	lavaEffect.initilize(SeaEffect::SeaEffectTypes::lava, XMINT2(100, 100), XMINT2(1, 1),
		float3(100.f, 2.f, 100.f), float3(100.f, 2.f, 100.f));
}

void LevelHandler::loadLevel(int levelNr) {
	if (m_currentLevel != levelNr) {
		m_currentLevel = levelNr;
		Level currentLevel = m_levelsArr.at(levelNr);

		m_terrainProps.load(currentLevel.m_terrainPropsFilename);

		for (int i = 0; i < m_levelsArr.at(levelNr).m_heightMapNames.size(); i++) {
			m_terrainManager->add(currentLevel.m_heightMapPos.at(i),
				currentLevel.m_heightMapScales[i], currentLevel.m_heightMapNames.at(i),
				currentLevel.m_heightmapTextures[i], currentLevel.m_heightMapSubSize.at(i),
				currentLevel.m_heightMapDivision.at(i));
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

		if (currentLevel.m_nrOfFruits[APPLE] != 0)
			m_hud.createFruitSprite("apple");
		if (currentLevel.m_nrOfFruits[BANANA] != 0)
			m_hud.createFruitSprite("banana");
		if (currentLevel.m_nrOfFruits[MELON] != 0)
			m_hud.createFruitSprite("melon");

		// m_entity.load("Sphere"); // castray debug don't delete
		// m_entity.setScale(0.1f);
		// m_entity.setPosition(float3(-2.f));
	}
	PathFindingThread::getInstance()->initialize(m_fruits, m_frame, m_collidableEntities);
}

void LevelHandler::draw() {
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate();
	}
	m_terrainManager->draw();

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->draw();
	}
	m_entity.draw();
	m_terrainProps.draw();
	m_skyBox.draw(m_oldTerrain, m_currentTerrain);


	// water/lava effect
	Renderer::getInstance()->copyDepthToSRV();
	waterEffect.draw();
	lavaEffect.draw();

	m_player.draw(); // draw after water/lava effect, bow will affect the depth buffer

	m_hud.draw();
}

void LevelHandler::update(float dt) {
	m_terrainProps.update(dt, m_player.getCameraPosition(), m_player.getForward());

	m_skyBox.updateDelta(dt);

	if (Input::getInstance()->keyPressed(Keyboard::R) && m_currentLevel >= 0)
		m_player.setPosition(m_levelsArr[m_currentLevel].m_playerStartPos);

	m_player.update(dt, m_terrainManager->getTerrainFromPosition(m_player.getPosition()));

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
		}
	}
	// update Pathfinding
	/*
	int batch = frameCounter++ * batchsize;
	for (auto f : m_fruits) {
		f.pathfinding();
	}
	*/
	auto pft = PathFindingThread::getInstance();

	// update stuff
	for (int i = 0; i < m_fruits.size(); i++) {
		pft->m_mutex.lock();
		m_fruits[i]->update(dt, playerPos, m_collidableEntities);
		if (m_player.isShooting()) {
			if (m_player.getArrow().checkCollision(*m_fruits[i])) {
				m_fruits[i]->hit();
				AudioHandler::getInstance()->playOnceByDistance(
					AudioHandler::HIT_FRUIT, m_player.getPosition(), m_fruits[i]->getPosition());

				m_player.getArrow().setPosition(
					float3(-100.f)); // temporary to disable arrow until returning
				ErrorLogger::log("Hit a fruit");
			}
		}
		if (m_fruits[i]->getState() == AI::State::CAUGHT) {
			if (float3(m_fruits[i].get()->getPosition() - m_player.getPosition()).Length() <
				1.0f) { // If the fruit is close to the player get picked up
				// pft->m_mutex.lock();
				pickUpFruit(m_fruits[i].get()->getFruitType());
				AudioHandler::getInstance()->playOnce(AudioHandler::COLLECT);
				m_fruits.erase(m_fruits.begin() + i);
				// pft->m_mutex.unlock();
			}
		}
		pft->m_mutex.unlock();
	}

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		pft->m_mutex.lock();
		m_player.collideObject(*m_collidableEntities[i]);
		pft->m_mutex.unlock();
	}
	// m_player.collideObject(*m_collidableEntities[1]);

	// castray sphere	// Debug thing will need later as well please don't delete - Linus
	// for (int i = 0; i < 3; ++i) {
	//	float t =
	//		m_collidableEntities[i]->castRay(m_player.getCameraPosition(), m_player.getForward());
	//	if (t != -1) {
	//		float3 tem = m_collidableEntities[i]->getHalfSizes();
	//		m_entity.setPosition(m_player.getCameraPosition() + t * m_player.getForward() * 0.9);
	//	}
	//}

	m_hud.update(dt);
	waterEffect.update(dt);
	lavaEffect.update(dt);
}

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
