#include "LevelHandler.h"
#include "TerrainManager.h"

LevelHandler::LevelHandler() { initialise(); }

LevelHandler::~LevelHandler() {}

void LevelHandler::initialise() {

	m_player.initialize();
	m_terrainManager = TerrainManager::getInstance();
	Level level0;

	level0.m_terrainTags.push_back(Level::TerrainTags::Forest);
	level0.m_terrainTags.push_back(Level::TerrainTags::Desert);
	level0.m_terrainTags.push_back(Level::TerrainTags::Volcano);
	level0.m_terrainTags.push_back(Level::TerrainTags::Plains);

	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");

	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(10.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 10.f));
	level0.m_heightMapPos.push_back(float3(10.f, 0.f, 10.f));

	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));

	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));

	level0.m_nrOfFruits[APPLE] = 1;
	level0.m_nrOfFruits[BANANA] = 100;
	level0.m_nrOfFruits[MELON] = 1;

	level0.m_playerStartPos = float3(5.f, 0.0f, 5.0f);

	level0.m_fruitPos[APPLE] = float3(5.0f, 0.0f, 0.0f);
	level0.m_fruitPos[BANANA] = float3(0.0f, 0.0f, 5.0f);
	level0.m_fruitPos[MELON] = float3(5.0f, 0.0f, 5.0f);

	m_levelsArr.push_back(level0);
}

void LevelHandler::loadLevel(int levelNr) {
	if (m_currentLevel != levelNr) {
		m_currentLevel = levelNr;
		Level currentLevel = m_levelsArr.at(levelNr);

		for (int i = 0; i < m_levelsArr.at(levelNr).m_heightMapNames.size(); i++) {
			m_terrainManager->add(currentLevel.m_heightMapPos.at(i),
				currentLevel.m_heightMapNames.at(i), currentLevel.m_heightMapSubSize.at(i),
				currentLevel.m_heightMapDivision.at(i));
		}

		for (int i = 0; i < currentLevel.m_nrOfFruits[APPLE]; i++) {
			shared_ptr<Apple> apple = make_shared<Apple>(currentLevel.m_fruitPos[APPLE]);
			m_fruits.push_back(apple);
		}
		for (int i = 0; i < currentLevel.m_nrOfFruits[BANANA]; i++) {
			shared_ptr<Banana> banana = make_shared<Banana>(currentLevel.m_fruitPos[BANANA]);
			m_fruits.push_back(banana);
		}
		for (int i = 0; i < currentLevel.m_nrOfFruits[MELON]; i++) {
			shared_ptr<Melon> melon = make_shared<Melon>(currentLevel.m_fruitPos[MELON]);
			m_fruits.push_back(melon);
		}

		m_player.setPosition(currentLevel.m_playerStartPos);

		m_currentTerrain = currentLevel.m_terrainTags[m_terrainManager->getTerrainIndexFromPosition(
			currentLevel.m_playerStartPos)];
	}
}

void LevelHandler::draw() {
	m_player.draw();
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate();
	}
	m_terrainManager->draw();
	m_skyBox.draw();
}

void LevelHandler::update(float dt) {
	m_player.update(dt, m_terrainManager->getTerrainFromPosition(m_player.getPosition()));

	dropFruit();

	float3 playerPos = m_player.getPosition();
	// update terrain tag
	int activeTerrain = m_terrainManager->getTerrainIndexFromPosition(playerPos);
	if (activeTerrain != -1 && m_currentLevel != -1) {
		Level::TerrainTags tag = m_levelsArr[m_currentLevel].m_terrainTags[activeTerrain];
		m_currentTerrain = tag;
	}

	// update stuff
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->update(dt, playerPos);
		m_fruits[i]->updateAnimated(dt);
		if (m_player.getArrow().checkCollision(*m_fruits[i])) {
			m_fruits[i]->setPosition(float3(0.f));
			m_player.getArrow().setPosition(float3(-10.f));
		}
		if (float3(m_fruits[i].get()->getPosition() - m_player.getPosition()).Length() <
			1.0f) { // If the fruit is close to the player get picked up
			pickUpFruit(m_fruits[i].get()->getFruitType());
			m_fruits.erase(m_fruits.begin() + i);
		}
	}
}

void LevelHandler::pickUpFruit(int fruitType) { m_inventory[fruitType]++; }

void LevelHandler::dropFruit() {
	Input* ip = Input::getInstance();

	if (ip->keyPressed(Keyboard::D1)) {
		if (m_inventory[APPLE] > 0) {
			shared_ptr<Apple> apple =
				make_shared<Apple>(float3(m_player.getPosition() + m_player.getForward() * 3.0f));
			m_fruits.push_back(apple);
			m_inventory[APPLE]--;
		}
	}
	if (ip->keyPressed(Keyboard::D2)) {
		if (m_inventory[BANANA] > 0) {
			shared_ptr<Banana> banana =
				make_shared<Banana>(float3(m_player.getPosition() + m_player.getForward() * 3.0f));
			m_fruits.push_back(banana);
			m_inventory[BANANA]--;
		}
	}
	if (ip->keyPressed(Keyboard::D3)) {
		if (m_inventory[MELON] > 0) {
			shared_ptr<Melon> melon =
				make_shared<Melon>(float3(m_player.getPosition() + m_player.getForward() * 3.0f));

			m_fruits.push_back(melon);
			m_inventory[MELON]--;
		}
	}
}
