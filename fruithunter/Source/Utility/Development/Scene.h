#pragma once
#include "GlobalNamespaces.h"

#include "SeaEffect.h"
#include "Animal.h"
#include "TerrainBatch.h"
#include "ParticleSystem.h"
#include "EntityRepository.h"
#include "SkyBox.h"
#include "Player.h"

#include "Apple.h"
#include "Banana.h"
#include "DragonFruit.h"
#include "Melon.h"

#include "VariableSyncer.h"

/*
 * Handles the file information structure
*/
class SceneAbstactContent {
private:
	string folder = "";
	FileSyncer file; // contains all data other than from heightmaps and sea contents
	const string prePath = "assets/Scenes/";

public:
	// Heightmaps
	struct HeightmapContent {
		int areaTag;				// AreaTag !!
		string heightmapName;
		float3 position;
		float3 scale;
		XMINT2 subSize;
		XMINT2 division;
		string texture0, texture1, texture2, texture3;
		float3 wind;
		int canSpawnFruit[NR_OF_FRUITS];// boolean Value
		FileSyncer file;
	};
	vector<HeightmapContent> m_heightmapAreas;

	// Seas
	struct SeaContent {
		int type;				//SeaEffect::SeaEffectTypes !!
		float3 position;
		float3 scale;
		XMINT2 tiles;
		XMINT2 grids;
		FileSyncer file;
	};
	vector<SeaContent> m_seaAreas;

	// ParticleSystems
	struct ParticleSystemContent {
		int type;				//ParticleSystem::PARTICLE_TYPE !!
		float3 position;
		FileSyncer file;
	};
	vector<ParticleSystemContent> m_particleSystemContents;

	//entities
	string m_entityStorageFilename;

	// Playerinfo
	float3 m_playerStartPos;

	// Level utility info
	struct SceneUtilityInfo {
		int nrOfFruits[NR_OF_FRUITS] = { 0 };
		int winCondition[NR_OF_FRUITS] = { 0 };
		int timeTargets[NR_OF_TIME_TARGETS] = { 0 };
		int levelIndex = -1; // standard as -1. Will keep value if scene isnt a level.
		// shall not be filled in load call!
		// Is used in Scene class to handle captured fruit count.
		int gathered[NR_OF_FRUITS] = { 0 }; 
		//--------------------------------------
	} utility;

	void load(string folder);
	void save(string folder);

	SceneAbstactContent();
};

/*
 * Data container for scene manager. Shall be used to manipulate the data. Save and load mechanism. Handles active data.
*/
class Scene {
protected:
	SceneAbstactContent content;

	//-- Private Functions --

	void clear();

	void saveWin();

public:
	// Skybox
	SkyBox m_skyBox;

	// HeightMap
	TerrainBatch m_terrains;

	// Sea effects
	vector<SeaEffect> m_seaEffects;

	// Static world entities
	EntityRepository m_repository;

	// Arrows
	vector<shared_ptr<Arrow>> m_arrows;

	// ParticleSystems
	vector<ParticleSystem> m_particleSystems;

	// Animals
	std::vector<Animal> m_animals;

	// Fruits
	std::vector<shared_ptr<Fruit>> m_fruits;

	// Playerinfo
	float3 m_playerStartPos;

	// Level utility info
	vector<int> m_fruitSpawnAreas[NR_OF_FRUITS];//holds index of terrains to spawn on
	SceneAbstactContent::SceneUtilityInfo m_utility;

	//timer
	Timer m_timer;

	//player
	shared_ptr<Player> m_player;

	//-- Private Functions --
	void resetPlayer();
	
public:

	Scene(string filename = "");
	~Scene();

	void pickUpFruit(FruitType fruitType);
	void dropFruit(FruitType fruitType);

	void load(string folder);
	void save(string folder);

	bool handleWin();

};
