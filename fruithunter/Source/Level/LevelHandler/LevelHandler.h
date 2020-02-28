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

#define LEVELS = 1;


struct PathFindingThread {
	size_t m_ObjectsToBeUpdated = 0;
	std::shared_ptr<size_t> m_currentFrame;
	thread* m_thread = nullptr;
	mutex m_mutex;
	std::vector<shared_ptr<Fruit>> m_batch;
	std::vector<shared_ptr<Entity>> m_collidables;


	bool checkVolitale(bool statement) {
		bool rtn;
		m_mutex.lock();
		rtn = statement;
		m_mutex.unlock();
	}


	void run() {

		size_t counter = 0;
		size_t index = 0;
		// Thread updateLoop
		while (1) {
			/*m_mutex.lock();
			counter += (*m_currentFrame % 10 == 0) ? 1 : 0;
			m_mutex.unlock();

			if (checkVolitale(m_batch.at(index)->giveNewPath())) {
				float3 pathStart = m_batch.at(index)->getPosition();
				m_batch.at(index)->pathfinding(pathStart, m_collidables, m_mutex);
			}
*/
			ErrorLogger::log("Thread running");
			index = (index < m_batch.size()) ? index + 1 : 0;
		}
	}

	PathFindingThread(std::vector<shared_ptr<Fruit>> batch, shared_ptr<size_t> currentFrame,
		vector<shared_ptr<Entity>> collidables) {
		m_batch = batch;
		m_currentFrame = currentFrame;
		m_collidables = collidables;
		m_thread = new thread([this] { run(); });
	}
};


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

	int m_currentLevel = -1;
	Level::TerrainTags m_currentTerrain = Level::TerrainTags::Forest;
	Level::TerrainTags m_oldTerrain = Level::TerrainTags::Forest;

	// HUD stuff
	HUD m_hud;

	// Creating data for levels.
	void initialiseLevel0();
	void placeBridge(float3 pos, float3 rot, float3 scale);
	void placeAllBridges();


	// thread for pathfinding,
	unique_ptr<PathFindingThread> m_thread;
	shared_ptr<size_t> m_frame;

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