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
#include "EntityRepository.h"
#include "HUD.h"
#include "SeaEffect.h"
#include "Animal.h"

#define LEVELS = 1;



struct Level {
	// HeightMap
	enum TerrainTags { Forest, Desert, Plains, Volcano, Count };
	vector<TerrainTags> m_terrainTags;
	std::vector<std::string> m_heightMapNames;
	std::vector<float3> m_heightMapPos;
	std::vector<XMINT2> m_heightMapSubSize;
	std::vector<XMINT2> m_heightMapDivision;
	std::vector<float3> m_heightMapScales;
	std::vector<std::vector<string>> m_heightmapTextures;

	// terrain props
	string m_terrainPropsFilename;

	// Wind
	vector<float3> m_wind;

	// Fruits
	int m_nrOfFruits[NR_OF_FRUITS];
	vector<int> m_fruitPos[NR_OF_FRUITS];

	// Playerinfo
	float3 m_playerStartPos;

	// Level utility info
	int m_winCondition[NR_OF_FRUITS];
	int m_timeTargets[NR_OF_TIME_TARGETS];
};

class LevelHandler {
private:
	std::vector<Level> m_levelsArr;
	std::vector<shared_ptr<Fruit>> m_fruits;
	Player m_player;
	TerrainManager* m_terrainManager;
	EntityRepository m_terrainProps;
	SeaEffect waterEffect, lavaEffect;
	Entity m_entity;
	SkyBox m_skyBox;
	size_t m_inventory[NR_OF_FRUITS]; // APPLE 0, BANANA 1, MELON 2
	vector<shared_ptr<Entity>> m_collidableEntities;
	vector<shared_ptr<Animal>> m_Animals;

	int m_currentLevel = -1;
	Level::TerrainTags m_currentTerrain = Level::TerrainTags::Forest;
	Level::TerrainTags m_oldTerrain = Level::TerrainTags::Forest;

	// HUD stuff
	HUD m_hud;

	// Creating data for levels.
	void initialiseLevel0();
	void placeBridge(float3 pos, float3 rot, float3 scale);
	void placeAllBridges();
	void placeAllAnimals();

	// thread for pathfinding,
	//unique_ptr<PathFindingThread> m_thread;
	shared_ptr<size_t> m_frame;

public:
	LevelHandler();
	~LevelHandler();

	void initialise();
	void loadLevel(int levelNr);
	void draw();
	void drawShadowDynamic();
	void drawShadowStatic();
	void drawShadowDynamicEntities();
	void update(float dt);

	// Fruit stuff
	void pickUpFruit(int fruitType);
	void dropFruit();

	std::vector<Fruit> fruitsInLevel;

	float3 getPlayerPos();
};