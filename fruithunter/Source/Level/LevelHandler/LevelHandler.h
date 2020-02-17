#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"
#include "Entity.h"
#include "TerrainManager.h"
#include "Apple.h"
#include "SkyBox.h"
#include "Banana.h"
#include "Melon.h"
#include "Player.h"

#define LEVELS = 1;

struct Level {
	// HeightMap
	std::vector<std::string> m_heightMapNames;
	std::vector<float3> m_heightMapPos;
	std::vector<XMINT2> m_heightMapSubSize;
	std::vector<XMINT2> m_heightMapDivision;
	std::vector<std::vector<string>> m_heightmapTextures;

	// Fruits
	int m_nrOfFruits[NR_OF_FRUITS];
	float3 m_fruitPos[NR_OF_FRUITS];

	// Playerinfo
	float3 m_playerStartPos;

	// Level utility info
	int m_winCondition[NR_OF_FRUITS];
};

class LevelHandler {
private:
	std::vector<Level> m_levelsArr;
	std::vector<shared_ptr<Fruit>> m_fruits;
	Player m_player;
	TerrainManager m_terrainManager;
	Entity m_entity;
	SkyBox m_skyBox;
	
public:
	LevelHandler();
	~LevelHandler();

	void initialise();
	void loadLevel(int levelNr);
	void draw();
	void update(float dt);
	

	std::vector<Fruit> fruitsInLevel;
};