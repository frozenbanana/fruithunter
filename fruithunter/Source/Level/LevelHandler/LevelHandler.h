#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"
#include "Entity.h"
#include "TerrainManager.h"
#include "Apple.h"
#include "SkyBox.h"
#include "Banana.h"
#include "Melon.h"
#include "DragonFruit.h"
#include "Player.h"
#include "EntityRepository.h"
#include "HUD.h"
#include "SeaEffect.h"
#include "QuadTree.h"
#include "Animal.h"
#include "ParticleSystem.h"

#define LEVELS = 3;

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

	//sea
	vector<SeaEffect::SeaEffectTypes> m_seaTypes;
	std::vector<float3> m_seaPositions;
	std::vector<XMINT2> m_seaTiles;
	std::vector<XMINT2> m_seaGrids;
	std::vector<float3> m_seaScales;

	// terrain props
	string m_terrainPropsFilename;

	//Bridges
	std::vector<float3> m_bridgePosition;
	std::vector<float3> m_bridgeRotation;
	std::vector<float3> m_bridgeScale;

	//Animals
	std::vector<shared_ptr<Animal>> m_animal;

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
	Entity m_sphere;
	Entity m_entity;
	SkyBox m_skyBox;
	size_t m_inventory[NR_OF_FRUITS]; // APPLE 0, BANANA 1, MELON 2
	vector<shared_ptr<Entity>> m_collidableEntities;
	vector<ParticleSystem> m_particleSystems;
	vector<shared_ptr<Animal>> m_Animals;
	vector<SeaEffect> m_seaEffects;

	QuadTree<int> tree;

	int m_currentLevel = -1;
	Level::TerrainTags m_currentTerrain = Level::TerrainTags::Forest;
	Level::TerrainTags m_oldTerrain = Level::TerrainTags::Forest;

	// HUD stuff
	HUD m_hud;

	// Creating data for levels.
	void initialiseLevel0();
	void initialiseLevel1();
	void initialiseLevel2();
	void placeBridge(float3 pos, float3 rot, float3 scale);
	//void placeAllBridges();
	//void placeAllAnimals();

	shared_ptr<size_t> m_frame;
	vector<FrustumPlane> m_planesTest;

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
	// To see if we have reached goal
	HUD& getHUD();
	// Fruit stuff
	void pickUpFruit(FruitType fruitType);
	void dropFruit();

	std::vector<Fruit> fruitsInLevel;

	float3 getPlayerPos();
	CubeBoundingBox getPlayerFrustumBB();
	vector<float3> getPlayerFrustumPoints(float scaleBetweenNearAndFarPlane);
};