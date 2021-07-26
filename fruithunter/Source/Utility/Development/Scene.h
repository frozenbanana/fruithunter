#pragma once
#include "GlobalNamespaces.h"

#include "AudioController.h"
#include "SeaEffect.h"
#include "TerrainBatch.h"
#include "ParticleSystem.h"
#include "EffectSystem.h"
#include "SkyBox.h"
#include "Player.h"
#include "CollectionPoint.h"
#include "RPYCamera.h"
#include "WorldMessage.h"

#include "Apple.h"
#include "Banana.h"
#include "DragonFruit.h"
#include "Melon.h"
#include "Pomegranate.h"

/*
 * Handles the file information structure
*/
class SceneAbstactContent {
private:
	string folder = "";
	const string path_scenes = "assets/Scenes/";

public:
	// leaderboard linked
	string m_leaderboardName = "";

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
		string psName;
		float3 position;
		float3 size;
		bool affectedByWind;
		float emitRate;
		size_t capacity;
	};
	vector<ParticleSystemContent> m_particleSystemContents;

	// Effects
	vector<shared_ptr<EffectSystem>> m_effects;

	// world messages
	vector<shared_ptr<WorldMessage>> m_messages;

	//entities
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

	// Level utility info
	struct SceneUtilityInfo {
		int winCondition[NR_OF_FRUITS] = { 0 };
		time_t timeTargets[NR_OF_TIME_TARGETS] = { 0 };
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
	string m_leaderboardName = "";

	RPYCamera m_camera;

	double m_deltaTime = 0, m_totalTime = 0;

	// Skybox
	SkyBox m_skyBox;

	// HeightMap
	TerrainBatch m_terrains;

	// Sea effects
	vector<shared_ptr<SeaEffect>> m_seaEffects;

	// Static world entities
	QuadTree<shared_ptr<Entity>> m_entities = QuadTree<shared_ptr<Entity>>(float3(0.), float3(200, 100, 200), 8);

	// messages
	vector<shared_ptr<WorldMessage>> m_worldMessages;

	// Arrows
	vector<shared_ptr<Arrow>> m_arrows;
	vector<shared_ptr<EffectSystem>> m_arrowParticles;

	// ParticleSystems
	vector<shared_ptr<ParticleSystem>> m_particleSystems;

	// effects
	vector<shared_ptr<EffectSystem>> m_effects;

	// Fruits
	std::vector<shared_ptr<Fruit>> m_fruits;

	// Collection points
	vector<shared_ptr<CollectionPoint>> m_collectionPoint;

	// Level utility info
	SceneAbstactContent::SceneUtilityInfo m_utility;
	int m_gatheredFruits[NR_OF_FRUITS] = { 0 }; 

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

	void addArrow(shared_ptr<Arrow> arrow);

	static SoundID playMusicByAreaTag(AreaTag tag);
	void update_activeTerrain(AreaTag tag, bool playMusic = true);

	void load(string folder);
	void save();

	/* Make scene ready to be played. Resets all dynamic content and keeps the static. */
	void reset();
	void clear();

	static TimeTargets getTimeTargetGrade(time_t timeMs, time_t timeTargets[NR_OF_TIME_TARGETS]);
	bool handleWin();

	float getDeltaTime() const;
	float getDeltaTime_skipSlow() const;
	size_t getTimePassedAsMilliseconds() const;
	double getTimePassedAsSeconds() const;
	void updateTime(double dt);

	void imgui_readProperties() const;

};
