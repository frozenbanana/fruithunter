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
	enum TerrainTags { Forest, Desert, Plains, Volcano, Count };
	vector<TerrainTags> m_terrainTags;
	std::vector<std::string> m_heightMapNames;
	std::vector<float3> m_heightMapPos;
	std::vector<XMINT2> m_heightMapSubSize;
	std::vector<XMINT2> m_heightMapDivision;

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
	TerrainManager* m_terrainManager;
	Entity m_entity;
	SkyBox m_skyBox;
	size_t m_inventory[NR_OF_FRUITS]; // APPLE 0, BANANA 1, MELON 2
	vector<shared_ptr<Entity>> m_collidableEntities;

	int m_currentLevel = -1;
	Level::TerrainTags m_currentTerrain = Level::TerrainTags::Forest;

public:
	LevelHandler();
	~LevelHandler();

	void initialise();
	void loadLevel(int levelNr);
	void draw();
	void update(float dt);

	// Fruit stuff
	void pickUpFruit(int fruitType);
	void dropFruit();


	std::vector<Fruit> fruitsInLevel;
};