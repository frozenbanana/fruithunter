#pragma once
#include "GlobalNamespaces.h"

#include "AudioController.h"
#include "SeaEffect.h"
#include "Animal.h"
#include "TerrainBatch.h"
#include "ParticleSystem.h"
#include "EntityRepository.h"
#include "SkyBox.h"
#include "Player.h"
#include "CollectionPoint.h"
#include "TotemLock.h"

#include "Apple.h"
#include "Banana.h"
#include "DragonFruit.h"
#include "Melon.h"

/*
 * Handles the file information structure
*/
class SceneAbstactContent {
private:
	string folder = "";
	const string path_scenes = "assets/Scenes/";

	static void fileInput_string(fstream& file, string str);
	static string fileOutput_string(fstream& file);
	static void fileInput_ulong(fstream& file, size_t v);
	static size_t fileOutput_ulong(fstream& file);

public:
	// Heightmaps
	struct HeightmapContent {
		int areaTag;				// AreaTag !!
		string heightmapName;
		float3 position;
		float3 rotation;
		float3 scale;
		XMINT2 subSize;
		XMINT2 division;
		string textures[4];
		float3 wind;
		int spawnFruits[NR_OF_FRUITS];// boolean Value
	};
	vector<HeightmapContent> m_heightmapAreas;

	// Seas
	struct SeaContent {
		int type;				//SeaEffect::SeaEffectTypes !!
		float3 position;
		float3 rotation;
		float3 scale;
		XMINT2 tiles;
		XMINT2 grids;
	};
	vector<SeaContent> m_seaAreas;

	// ParticleSystems
	struct ParticleSystemContent {
		int type;				//ParticleSystem::PARTICLE_TYPE !!
		float3 position;
		float3 size;
		bool affectedByWind;
		size_t emitRate;
		size_t capacity;
	};
	vector<ParticleSystemContent> m_particleSystemContents;

	//entities
	string m_entityStorageFilename;
	struct GroupInstance {
		string model;
		struct Instance {
			float3 position, rotation, scale;
			bool collidable;
			Instance(float3 _position = float3(), float3 _rotation = float3(),
				float3 _scale = float3(), bool _collidable = true) {
				position = _position;
				rotation = _rotation;
				scale = _scale;
				collidable = _collidable;
			}
		};
		vector<Instance> instances;
		GroupInstance(string _model = "") { model = _model; }
	};
	vector<GroupInstance> m_entities;

	//animals
	struct AnimalContent {
		float3 position;
		float3 sleepPosition;
		int type; //Animal::Type;
		int fruitType; //FruitType
		int fruitCount;
		float rotationY;
	};
	vector<AnimalContent> m_animals;

	// Level utility info
	struct SceneUtilityInfo {
		int winCondition[NR_OF_FRUITS] = { 0 };
		int timeTargets[NR_OF_TIME_TARGETS] = { 0 };
		float3 startSpawn;
	} m_utility;

	bool load_raw(string folder);
	bool save_raw(string folder);

	SceneAbstactContent();
};

/*
 * Data container for scene manager. Shall be used to manipulate the data. Save and load mechanism. Handles active data.
*/
class Scene {
protected:
	bool m_loaded = false;

	//-- Private Functions --
	void saveWin();

public:
	string m_sceneName = "";

	// Skybox
	SkyBox m_skyBox;

	// HeightMap
	TerrainBatch m_terrains;

	// Sea effects
	vector<shared_ptr<SeaEffect>> m_seaEffects;

	// Static world entities
	QuadTree<shared_ptr<Entity>> m_entities = QuadTree<shared_ptr<Entity>>(float3(0.), float3(200, 100, 200), 4);

	// Arrows
	vector<shared_ptr<Arrow>> m_arrows;

	// ParticleSystems
	vector<ParticleSystem> m_particleSystems;

	// Animals
	std::vector<shared_ptr<Animal>> m_animals;

	// Totems
	vector<shared_ptr<TotemLock>> m_totems;

	// Fruits
	std::vector<shared_ptr<Fruit>> m_fruits;

	// Collection points
	vector<shared_ptr<CollectionPoint>> m_collectionPoint;

	// Level utility info
	struct FruitCombo {
		int quantity[NR_OF_FRUITS] = { 0 };
	};
	vector<FruitCombo> m_fruitSpawns;
	SceneAbstactContent::SceneUtilityInfo m_utility;
	int m_gatheredFruits[NR_OF_FRUITS] = { 0 }; 

	//timer
	Timer m_timer;

	// active terrain
	AreaTag m_activeTerrain_tag = AreaTag::Plains;
	SoundID m_activeTerrain_soundID = 0;

	//player
	shared_ptr<Player> m_player;

	//-- Private Functions --
	
public:

	Scene(string filename = "");
	~Scene();

	size_t find_parentIndex(Fragment* fragment);
	bool remove_fragment(Fragment* fragment);
	void updated_fragment(Fragment* fragment);

	void pickUpFruit(FruitType fruitType);
	void dropFruit(FruitType fruitType);

	static SoundID playMusicByAreaTag(AreaTag tag);
	void update_activeTerrain(AreaTag tag);

	void load(string folder);
	void save();

	/* Make scene ready to be played. Resets all dynamic content and keeps the static. */
	void reset();
	void clear();

	bool handleWin();

	float getDeltaTime();
	float getDeltaTime_skipSlow();

};
