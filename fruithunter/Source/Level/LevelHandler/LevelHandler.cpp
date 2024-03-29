#include "LevelHandler.h"
#include "TerrainManager.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "VariableSyncer.h"
#include "PathFindingThread.h"


#define WIND_PLAINS float3(0.f, 0.f, 7.f)
#define WIND_FOREST float3(3.f, 0.f, -5.f)
#define WIND_DESERT float3(-5.f, 0.f, 4.f)
#define WIND_VULCANO float3(0.f, 8.f, 0.f)

void LevelHandler::initialiseLevel0() {
	Level level;

	level.m_terrainPropsFilename = "level0";

	level.m_terrainTags.push_back(Level::TerrainTags::Plains);

	level.m_fruitPos[APPLE].push_back(0);

	level.m_heightMapNames.push_back("PlainMap.png");

	level.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));

	level.m_heightMapSubSize.push_back(XMINT2(15, 15));

	level.m_heightMapDivision.push_back(XMINT2(16, 16));

	level.m_heightMapScales.push_back(float3(1.f, 0.10f, 1.f) * 100);

	vector<string> maps(4);
	maps[0] = "texture_grass.jpg";
	maps[1] = "texture_rock4.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level.m_heightmapTextures.push_back(maps);

	level.m_seaTypes.push_back(SeaEffect::SeaEffectTypes::water);
	level.m_seaPositions.push_back(
		float3(0.f, 1.f, 0.f) - float3(200.f, 0, 200.f) + float3(50.f, 0, 50.f));
	level.m_seaScales.push_back(float3(400, 2, 400));
	level.m_seaGrids.push_back(XMINT2(8, 8));
	level.m_seaTiles.push_back(XMINT2(50, 50));

	// level.m_wind.push_back(float3(0.f, 8.f, 0.f)); // Volcano
	// level.m_wind.push_back(float3(3.f, 0.f, 6.f)); // Forest
	// level.m_wind.push_back(float3(1.f, 0.f, 2.f)); // Desert
	level.m_wind.push_back(float3(0.f, 0.f, 3.f)); // Plains

	level.m_nrOfFruits[APPLE] = 30;
	level.m_nrOfFruits[BANANA] = 0;
	level.m_nrOfFruits[MELON] = 0;
	level.m_nrOfFruits[DRAGON] = 0;

	level.m_winCondition[APPLE] = 15;
	level.m_winCondition[BANANA] = 0;
	level.m_winCondition[MELON] = 0;
	level.m_winCondition[DRAGON] = 0;

	level.m_playerStartPos = float3(5.6f, 2.5f, 38.7f);

	level.m_timeTargets[GOLD] = 30;
	level.m_timeTargets[SILVER] = 40;
	level.m_timeTargets[BRONZE] = 60;

	m_levelsArr.push_back(level);
	m_hud.setTimeTargets(level.m_timeTargets);
}

void LevelHandler::initialiseLevel1() {
	Level level;

	level.m_terrainPropsFilename = "level1";

	level.m_terrainTags.push_back(Level::TerrainTags::Forest);
	level.m_terrainTags.push_back(Level::TerrainTags::Plains);

	level.m_fruitPos[APPLE].push_back(0);
	level.m_fruitPos[MELON].push_back(1);
	level.m_fruitPos[DRAGON].push_back(0);


	level.m_heightMapNames.push_back("ForestMap.png");
	level.m_heightMapNames.push_back("PlainMap.png");

	level.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level.m_heightMapPos.push_back(float3(100.f, 0.f, 0.f));

	level.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level.m_heightMapSubSize.push_back(XMINT2(15, 15));

	level.m_heightMapDivision.push_back(XMINT2(16, 16));
	level.m_heightMapDivision.push_back(XMINT2(16, 16));

	level.m_heightMapScales.push_back(float3(1.f, 0.15f, 1.f) * 100);
	level.m_heightMapScales.push_back(float3(1.f, 0.10f, 1.f) * 100);

	vector<string> maps(4);
	maps[0] = "texture_grass3.jpg";
	maps[1] = "texture_sand1.jpg";
	maps[2] = "texture_mossyRock.jpg";
	maps[3] = "texture_mossyRock.jpg";
	level.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass.jpg";
	maps[1] = "texture_rock4.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level.m_heightmapTextures.push_back(maps);

	level.m_seaTypes.push_back(SeaEffect::SeaEffectTypes::water);
	level.m_seaPositions.push_back(
		float3(0.f, 1.f, 0.f) - float3(200.f, 0.f, 200.f) + float3(100.f, 0, 50.f));
	level.m_seaScales.push_back(float3(400.f, 2.f, 400.f));
	level.m_seaGrids.push_back(XMINT2(8, 8));
	level.m_seaTiles.push_back(XMINT2(50, 50));

	level.m_bridgePosition.push_back(float3(103.2f, 3.1f, 39.f));
	// level.m_bridgePosition.push_back(float3(35.f, 3.5f, 98.5f));

	level.m_bridgeRotation.push_back(float3(0.f, -0.1f, -0.07f));
	// level.m_bridgeRotation.push_back(float3(0.f, 1.7f, 0.02f));

	level.m_bridgeScale.push_back(float3(1.9f, 1.f, 1.4f));
	// level.m_bridgeScale.push_back(float3(1.6f, 1.f, 1.4f));

	shared_ptr<Animal> animal = make_shared<Animal>("Goat", 10.f, 7.f, FruitType::APPLE, 2, 10.f,
		float3(96.2f, 3.45f, 38.f), float3(90.2f, 3.7f, 49.f), XM_PI * 0.5f);
	level.m_animal.push_back(animal);

	level.m_wind.push_back(WIND_FOREST); // Forest
	level.m_wind.push_back(WIND_PLAINS); // Plains

	level.m_nrOfFruits[APPLE] = 20;
	level.m_nrOfFruits[BANANA] = 0;
	level.m_nrOfFruits[MELON] = 20;
	level.m_nrOfFruits[DRAGON] = 0;

	level.m_winCondition[APPLE] = 10;
	level.m_winCondition[BANANA] = 0;
	level.m_winCondition[MELON] = 10;
	level.m_winCondition[DRAGON] = 0;


	level.m_playerStartPos = float3(5.9f, 3.2f, 74.4f);

	level.m_timeTargets[GOLD] = 70;
	level.m_timeTargets[SILVER] = 90;
	level.m_timeTargets[BRONZE] = 120;

	m_levelsArr.push_back(level);
	m_hud.setTimeTargets(level.m_timeTargets);
}

void LevelHandler::initialiseLevel2() {
	Level level;

	level.m_terrainPropsFilename = "level2";

	level.m_terrainTags.push_back(Level::TerrainTags::Volcano); // 1
	level.m_terrainTags.push_back(Level::TerrainTags::Forest);	// 2
	level.m_terrainTags.push_back(Level::TerrainTags::Desert);	// 3
	level.m_terrainTags.push_back(Level::TerrainTags::Plains);	// 4

	level.m_fruitPos[APPLE].push_back(3); // 2 is  desert
	level.m_fruitPos[APPLE].push_back(1);
	level.m_fruitPos[BANANA].push_back(1); //  0 is volcano
	level.m_fruitPos[BANANA].push_back(2);
	level.m_fruitPos[BANANA].push_back(3);
	level.m_fruitPos[MELON].push_back(2);  // 1 is forest
	level.m_fruitPos[DRAGON].push_back(0); // 3 is plains

	level.m_heightMapNames.push_back("VolcanoMap.png");
	level.m_heightMapNames.push_back("ForestMap.png");
	level.m_heightMapNames.push_back("DesertMap.png");
	level.m_heightMapNames.push_back("PlainMap.png");

	level.m_heightMapPos.push_back(float3(100.f, 0.f, 100.f));
	level.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level.m_heightMapPos.push_back(float3(0.f, 0.f, 100.f));
	level.m_heightMapPos.push_back(float3(100.f, 0.f, 0.f));

	level.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level.m_heightMapSubSize.push_back(XMINT2(15, 15));
	level.m_heightMapSubSize.push_back(XMINT2(15, 15));

	level.m_heightMapDivision.push_back(XMINT2(16, 16));
	level.m_heightMapDivision.push_back(XMINT2(16, 16));
	level.m_heightMapDivision.push_back(XMINT2(16, 16));
	level.m_heightMapDivision.push_back(XMINT2(16, 16));

	level.m_heightMapScales.push_back(float3(1.f, 0.40f, 1.f) * 100);
	level.m_heightMapScales.push_back(float3(1.f, 0.15f, 1.f) * 100);
	level.m_heightMapScales.push_back(float3(1.f, 0.25f, 1.f) * 100);
	level.m_heightMapScales.push_back(float3(1.f, 0.10f, 1.f) * 100);

	vector<string> maps(4);
	maps[0] = "texture_rock8.jpg"; // flat
	maps[1] = "texture_lava1.jpg"; // low flat
	maps[2] = "texture_rock2.jpg"; // tilt
	maps[3] = "texture_rock2.jpg"; // low tilt
	level.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass3.jpg";
	maps[1] = "texture_sand1.jpg";
	maps[2] = "texture_mossyRock.jpg";
	maps[3] = "texture_mossyRock.jpg";
	level.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_sand3.jpg";
	maps[1] = "texture_sand1.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass.jpg";
	maps[1] = "texture_rock4.jpg";
	maps[2] = "texture_rock6.jpg";
	maps[3] = "texture_rock6.jpg";
	level.m_heightmapTextures.push_back(maps);

	level.m_seaTypes.push_back(SeaEffect::SeaEffectTypes::water);
	level.m_seaPositions.push_back(float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f));
	level.m_seaScales.push_back(float3(400.f, 2.f, 400.f));
	level.m_seaGrids.push_back(XMINT2(8, 8));
	level.m_seaTiles.push_back(XMINT2(50, 50));
	float3 lavaSize(82.f, 0.f, 82.f);
	float3 lavaPos(150, 1.5f, 150);
	level.m_seaTypes.push_back(SeaEffect::SeaEffectTypes::lava);
	level.m_seaPositions.push_back(lavaPos - lavaSize / 2.f);
	level.m_seaScales.push_back(lavaSize + float3(0, 2.f, 0));
	level.m_seaGrids.push_back(XMINT2(4, 4));
	level.m_seaTiles.push_back(XMINT2(20, 20));

	level.m_bridgePosition.push_back(float3(103.2f, 3.1f, 39.f));
	level.m_bridgePosition.push_back(float3(35.f, 3.5f, 98.5f));
	level.m_bridgePosition.push_back(float3(99.2f, 7.9f, 155.f));

	level.m_bridgeRotation.push_back(float3(0.f, -0.1f, -0.07f));
	level.m_bridgeRotation.push_back(float3(0.f, 1.7f, 0.02f));
	level.m_bridgeRotation.push_back(float3(0.f, 0.f, -0.09f));

	level.m_bridgeScale.push_back(float3(1.9f, 1.f, 1.4f));
	level.m_bridgeScale.push_back(float3(1.6f, 1.f, 1.4f));
	level.m_bridgeScale.push_back(float3(1.3f, 1.f, 1.4f));

	shared_ptr<Animal> animal = make_shared<Animal>("Gorilla", 10.f, 7.f, BANANA, 2, 10.f,
		float3(110.115f, 2.46f, 39.79f), float3(90.2f, 3.7f, 49.f), XM_PI * 0.5f);
	level.m_animal.push_back(animal);

	animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f, float3(37.f, 3.2f, 93.f),
		float3(20.f, 3.7f, 90.f), 0.f);
	level.m_animal.push_back(animal);

	animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f, float3(90.f, 8.2f, 152.f),
		float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
	level.m_animal.push_back(animal);

	level.m_wind.push_back(WIND_VULCANO); // Volcano
	level.m_wind.push_back(WIND_FOREST);  // Forest
	level.m_wind.push_back(WIND_DESERT);  // Desert
	level.m_wind.push_back(WIND_PLAINS);  // Plains

	level.m_nrOfFruits[APPLE] = 20;
	level.m_nrOfFruits[BANANA] = 20;
	level.m_nrOfFruits[MELON] = 20;
	level.m_nrOfFruits[DRAGON] = 10;

	level.m_winCondition[APPLE] = 5;
	level.m_winCondition[BANANA] = 5;
	level.m_winCondition[MELON] = 5;
	level.m_winCondition[DRAGON] = 5;
	level.m_playerStartPos = float3(162.5f, 9.5f, 19.f);

	level.m_timeTargets[GOLD] = 150;
	level.m_timeTargets[SILVER] = 180;
	level.m_timeTargets[BRONZE] = 210;

	m_levelsArr.push_back(level);
	m_hud.setTimeTargets(level.m_timeTargets);
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

LevelHandler::LevelHandler() { initialise(); }

LevelHandler::~LevelHandler() {
	PathFindingThread::getInstance()->exitThread();
	m_terrainManager->removeAll();
}

void LevelHandler::initialise() {
	m_sphere.load("Sphere");
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
	m_terrainProps.addPlaceableEntity("RopeBridgeFloor");
	m_terrainProps.addPlaceableEntity("RopeBridgeRailing1");
	m_terrainProps.addPlaceableEntity("RopeBridgeRailing2");
	m_terrainProps.addPlaceableEntity("SignSlanted");
	m_terrainProps.addPlaceableEntity("SignSlanted2");
	m_terrainProps.addPlaceableEntity("SignHorizontal");
	m_terrainProps.addPlaceableEntity("SignHorizontal2");

	initialiseLevel0();
	initialiseLevel1();
	initialiseLevel2();

	m_particleSystems.resize(6);
	m_particleSystems[0] = ParticleSystem(ParticleSystem::VULCANO_FIRE);
	m_particleSystems[0].setPosition(float3(150.f, 25.f, 150.f));
	m_particleSystems[1] = ParticleSystem(ParticleSystem::VULCANO_SMOKE);
	m_particleSystems[1].setPosition(float3(150.f, 30.f, 150.f));
	m_particleSystems[2] = ParticleSystem(ParticleSystem::GROUND_DUST);
	m_particleSystems[2].setPosition(float3(42.f, 4.f, 125.f));
	m_particleSystems[3] = ParticleSystem(ParticleSystem::FOREST_BUBBLE);
	m_particleSystems[3].setPosition(float3(50.f, 2.f, 40.f));
	m_particleSystems[4] = ParticleSystem(ParticleSystem::LAVA_BUBBLE);
	m_particleSystems[4].setPosition(float3(150.f, 0.f, 149.f));
	m_particleSystems[5] = ParticleSystem(ParticleSystem::GROUND_DUST);
	m_particleSystems[5].setPosition(float3(125.f, 4.f, 50.f));
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

		m_seaEffects.resize(m_levelsArr.at(levelNr).m_seaTypes.size());
		for (size_t i = 0; i < m_levelsArr.at(levelNr).m_seaTypes.size(); i++) {
			m_seaEffects[i].initilize(currentLevel.m_seaTypes[i], currentLevel.m_seaTiles[i],
				currentLevel.m_seaGrids[i], currentLevel.m_seaPositions[i],
				currentLevel.m_seaScales[i]);
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
		for (int i = 0; i < currentLevel.m_nrOfFruits[DRAGON]; i++) {
			int terrainTagNr = i % currentLevel.m_fruitPos[DRAGON].size();
			shared_ptr<DragonFruit> dragon = make_shared<DragonFruit>(
				m_terrainManager->getSpawnpoint(currentLevel.m_fruitPos[DRAGON].at(terrainTagNr)));
			m_fruits.push_back(dragon);
		}


		m_player.setPosition(currentLevel.m_playerStartPos);

		m_currentTerrain = currentLevel.m_terrainTags[m_terrainManager->getTerrainIndexFromPosition(
			currentLevel.m_playerStartPos)];
		m_skyBox.updateNewOldLight(m_currentTerrain);
		AudioHandler::getInstance()->changeMusicByTag(m_currentTerrain, 0);

		m_hud.setTimeTargets(currentLevel.m_timeTargets);
		m_hud.setWinCondition(currentLevel.m_winCondition);
		m_hud.setLevelIndex(levelNr);


		if (currentLevel.m_nrOfFruits[APPLE] != 0)
			m_hud.createFruitSprite("apple");
		if (currentLevel.m_nrOfFruits[BANANA] != 0)
			m_hud.createFruitSprite("banana");
		if (currentLevel.m_nrOfFruits[MELON] != 0)
			m_hud.createFruitSprite("melon");
		if (currentLevel.m_nrOfFruits[DRAGON] != 0)
			m_hud.createFruitSprite("dragonfruit");

		// Put out bridges correctly
		for (int i = 0; i < currentLevel.m_bridgePosition.size(); i++) {
			placeBridge(currentLevel.m_bridgePosition[i], currentLevel.m_bridgeRotation[i],
				currentLevel.m_bridgeScale[i]);
		}

		for (int i = 0; i < currentLevel.m_animal.size(); i++) {
			m_Animals.push_back(currentLevel.m_animal[i]);
		}
	}

	if (PathFindingThread::getInstance()->m_thread == nullptr) {
		std::vector<float4> animalPos;
		for (auto a : m_Animals) {
			float4 ani = float4(
				a->getPosition().x, a->getPosition().y, a->getPosition().z, a->getFruitRange());
			animalPos.push_back(ani);
		}
		PathFindingThread::getInstance()->initialize(m_fruits, m_frame, m_terrainProps, animalPos);
	}
}

void LevelHandler::draw() {
	m_skyBox.bindLightBuffer();
	m_player.draw();

	Renderer::getInstance()->enableAlphaBlending();
	for (int i = 0; i < m_fruits.size(); i++) {
		if (m_fruits[i]->isVisible()) {
			m_fruits[i]->draw_animate();
			m_fruits[i]->getParticleSystem()->drawNoAlpha();
		}
	}

	Renderer::getInstance()->disableAlphaBlending();

	for (size_t i = 0; i < m_Animals.size(); ++i) {
		m_Animals[i]->draw();
	}

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->draw();
	}
	m_entity.draw();

	// frustum data for culling
	vector<FrustumPlane> frustum = m_player.getFrustumPlanes();
	// terrain entities
	m_terrainProps.quadtreeCull(frustum);
	m_terrainProps.draw();

	// terrain
	m_terrainManager->quadtreeCull(frustum);
	m_terrainManager->draw();
	// water/lava effect
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < m_seaEffects.size(); i++) {
		m_seaEffects[i].quadtreeCull(frustum);
		m_seaEffects[i].draw();
	}

	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */
	// Particle Systems
	for (size_t i = 0; i < m_particleSystems.size(); i++) {
		m_particleSystems[i].draw();
	}
	m_player.getBow().getTrailEffect().draw();

	m_skyBox.draw(m_oldTerrain, m_currentTerrain);
	m_hud.draw(); // TODO: Find out why hud is not drawn if particleSystems are before
}

void LevelHandler::drawShadowDynamic() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate_onlyMesh(float3(0, 0, 0));
	}

	// terrain manager
	m_terrainManager->quadtreeCull(planes);
	m_terrainManager->draw_onlyMesh();

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->draw_onlyMesh(float3(0, 0, 0));
	}

	// terrain entities
	m_terrainProps.quadtreeCull(planes);
	m_terrainProps.draw_onlyMesh();
}

void LevelHandler::drawShadowStatic() {
	m_terrainManager->draw_onlyMesh();

	for (size_t i = 0; i < m_collidableEntities.size(); ++i) {
		m_collidableEntities[i]->draw_onlyMesh(float3(0, 0, 0));
	}

	m_terrainProps.draw_onlyMesh();
}

void LevelHandler::drawShadowDynamicEntities() {
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate_onlyMesh(float3(0, 0, 0));
	}
}

void LevelHandler::update(float dt) {
	auto pft = PathFindingThread::getInstance();

	m_terrainProps.update(dt, m_player.getCameraPosition(), m_player.getForward());

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
		m_Animals[i]->checkLookedAt(m_player.getCameraPosition(), m_player.getForward());
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
				if (m_fruits[iFruit]->getFruitType() == m_Animals[i]->getfruitType() &&
					(m_fruits[iFruit]->getState() == AI::State::RELEASED ||
						m_fruits[iFruit]->getState() == AI::State::CAUGHT)) {
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
	// ErrorLogger::logFloat3("Pos: ", playerPos);
	// update terrain tag
	int activeTerrain = m_terrainManager->getTerrainIndexFromPosition(playerPos);

	if (activeTerrain != -1 && m_currentLevel != -1) {
		Level::TerrainTags tag = m_levelsArr[m_currentLevel].m_terrainTags[activeTerrain];
		if (m_currentTerrain != tag) {
			m_oldTerrain = m_currentTerrain;
			m_currentTerrain = tag;
			m_skyBox.updateNewOldLight(tag);
			m_skyBox.resetDelta();
			AudioHandler::getInstance()->changeMusicByTag(tag, dt);
		}
	}

	m_skyBox.updateCurrentLight();

	// update stuff

	for (int i = 0; i < m_fruits.size(); i++) {

		m_fruits[i]->getParticleSystem()->update(dt);
		pft->m_mutex.lock();
		m_fruits[i]->update(dt, playerPos);
		if (m_player.isShooting()) {

			if (m_player.getArrow().checkCollision(*m_fruits[i])) {
				m_player.getStaminaBySkillshot(m_fruits[i]->hit(m_player.getPosition()));
				AudioHandler::getInstance()->playOnceByDistance(
					AudioHandler::HIT_FRUIT, m_player.getPosition(), m_fruits[i]->getPosition());

				m_player.getArrow().setPosition(
					float3(-999.f)); // temporary to disable arrow until returning
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
		if (currentTerrain != nullptr) {
			if (m_particleSystems[i].getType() == ParticleSystem::VULCANO_SMOKE ||
				m_particleSystems[i].getType() == ParticleSystem::VULCANO_FIRE ||
				m_particleSystems[i].getType() == ParticleSystem::LAVA_BUBBLE) {
				m_particleSystems[i].update(dt, currentTerrain->getWindStatic());
			}
			else {
				m_particleSystems[i].update(dt, currentTerrain); // Get wind dynamically
			}
		}
	}

	m_hud.update(dt, m_player.getStamina());
	for (size_t i = 0; i < m_seaEffects.size(); i++)
		m_seaEffects[i].update(dt);
}

HUD& LevelHandler::getHUD() { return m_hud; }

void LevelHandler::pickUpFruit(FruitType fruitType) {
	m_inventory[fruitType]++;
	m_hud.addFruit(fruitType);
}

void LevelHandler::dropFruit() {
	Input* ip = Input::getInstance();
	auto pft = PathFindingThread::getInstance();

	if (ip->keyPressed(Keyboard::D1)) {
		if (m_inventory[APPLE] > 0) {
			shared_ptr<Apple> apple =
				make_shared<Apple>(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f));
			apple->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(apple);
			pft->m_mutex.unlock();
			if (!DEBUG)
				m_inventory[APPLE]--;
			m_hud.removeFruit(APPLE);
		}
	}
	if (ip->keyPressed(Keyboard::D2)) {
		if (m_inventory[BANANA] > 0) {
			shared_ptr<Banana> banana =
				make_shared<Banana>(float3(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
			banana->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(banana);
			pft->m_mutex.unlock();
			if (!DEBUG)
				m_inventory[BANANA]--;
			m_hud.removeFruit(BANANA);
		}
	}
	if (ip->keyPressed(Keyboard::D3)) {
		if (m_inventory[MELON] > 0) {
			shared_ptr<Melon> melon =
				make_shared<Melon>((m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
			melon->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(melon);
			pft->m_mutex.unlock();
			if (!DEBUG)
				m_inventory[MELON]--;
			m_hud.removeFruit(MELON);
		}
	}
	if (ip->keyPressed(Keyboard::D4)) {
		if (m_inventory[DRAGON] > 0) {
			shared_ptr<DragonFruit> dragonFruit =
				make_shared<DragonFruit>((m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
			dragonFruit->release(m_player.getForward());
			pft->m_mutex.lock();
			m_fruits.push_back(dragonFruit);
			pft->m_mutex.unlock();
			if (!DEBUG)
				m_inventory[DRAGON]--;
			m_hud.removeFruit(DRAGON);
		}
	}
}

float3 LevelHandler::getPlayerPos() { return m_player.getPosition(); }

CubeBoundingBox LevelHandler::getPlayerFrustumBB() { return m_player.getCameraBoundingBox(); }

vector<float3> LevelHandler::getPlayerFrustumPoints(float scaleBetweenNearAndFarPlane) {
	return m_player.getFrustumPoints(scaleBetweenNearAndFarPlane);
}
