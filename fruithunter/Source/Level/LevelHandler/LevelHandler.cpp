#include "LevelHandler.h"
#include "TerrainManager.h"

LevelHandler::LevelHandler() {
	initialise();
}

LevelHandler::~LevelHandler() {
}

void LevelHandler::initialise() {

	m_player.initialize();

	Level level0;
	level0.m_heightMapNames.push_back("VolcanoMap.png");
	level0.m_heightMapNames.push_back("ForestMap.png");
	level0.m_heightMapNames.push_back("DesertMap.png");
	level0.m_heightMapNames.push_back("PlainMap.png");

	level0.m_heightMapPos.push_back(float3(100.f, 0.f, 100.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 100.f));
	level0.m_heightMapPos.push_back(float3(100.f, 0.f, 0.f));

	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));

	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));

	level0.m_heightMapScales.push_back(float3(1, 0.20, 1) * 100);
	level0.m_heightMapScales.push_back(float3(1, 0.15, 1) * 100);
	level0.m_heightMapScales.push_back(float3(1, 0.20, 1) * 100);
	level0.m_heightMapScales.push_back(float3(1, 0.10, 1) * 100);

	vector<string> maps(4);
	maps[0] = "texture_rock8_1jpg"; // flat
	maps[1] = "texture_lava1.jpg"; // low flat
	maps[2] = "texture_rock2.jpg"; // tilt
	maps[3] = "texture_rock2.jpg"; // low tilt
	level0.m_heightmapTextures.push_back(maps);
	maps[0] = "texture_grass2_1.jpg";
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

	level0.m_nrOfFruits[APPLE] = 1;
	level0.m_nrOfFruits[BANANA] = 1;
	level0.m_nrOfFruits[MELON] = 1;

	level0.m_playerStartPos = float3(110.f, 0.0f, 150.0f);

	level0.m_fruitPos[APPLE] = float3(5.0f, 0.0f, 0.0f);
	level0.m_fruitPos[BANANA] = float3(0.0f, 0.0f, 5.0f);
	level0.m_fruitPos[MELON] = float3(5.0f, 0.0f, 5.0f);

	m_levelsArr.push_back(level0);
}

void LevelHandler::loadLevel(int levelNr) {
	Level currentLevel = m_levelsArr.at(levelNr);

	for (int i = 0; i < m_levelsArr.at(levelNr).m_heightMapNames.size(); i++) {
		m_terrainManager.add(currentLevel.m_heightMapPos.at(i), currentLevel.m_heightMapScales[i],
			currentLevel.m_heightMapNames.at(i), currentLevel.m_heightmapTextures[i],
			currentLevel.m_heightMapSubSize.at(i), currentLevel.m_heightMapDivision.at(i));

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
}

void LevelHandler::draw() {
	m_player.draw();
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate();
	}
	m_terrainManager.draw();
	m_skyBox.draw();
}

void LevelHandler::update(float dt) {
	m_player.update(dt, m_terrainManager.getTerrainFromPosition(m_player.getPosition()));

	if(Input::getInstance()->keyPressed(Keyboard::R))m_player.setPosition(m_levelsArr[0].m_playerStartPos);

	float3 playerPos = m_player.getPosition();

	for (int i = 0; i < m_fruits.size(); i++) {
		//m_fruits[i]->update(dt, playerPos);
		m_fruits[i]->updateAnimated(dt);
		/*if (m_player.getArrow().checkCollision(m_fruits[i])) {
			m_fruits[i].setPosition(float3(0.f));
			m_player.getArrow().setPosition(float3(-1.f));
		}*/
	}
}
