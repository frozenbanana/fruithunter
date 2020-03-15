#pragma once
#include "GlobalNameSpaces.h"
#include "SeaEffect.h"
#include "TerrainManager.h"
#include "Animated.h"
#include "SkyBox.h"
#include "Fruits.h"
#include "ParticleSystem.h"
#include "Animal.h"
#include "Player.h"
#include "QuadTree.h"
#include "PathFindingThread.h"
#include "AudioHandler.h"

enum SceneEvent {
	event_none,
	event_pickUp_Apple,
	event_pickUp_Banana,
	event_pickUp_Melon,
	event_changeLevel
};

class Scene {
private:
	//EDITOR STUFF
	//const string m_entityPlacementFilePath = "assets/EntityPlacements/";
	//const string m_fileEndings = ".teps"; // tep = Terrain Entity Placements

	//// modifiers for collision
	//const vector<string> m_treeNames = { "treeMedium1", "treeMedium2", "treeMedium3", "BurnedTree1",
	//	"BurnedTree2", "BurnedTree3" };
	//const vector<string> m_nonCollidables = { "DeadBush", "Grass1", "Grass2", "Grass3", "Grass4" };

	//// placeable stuff
	//vector<unique_ptr<Entity>> m_placeable; // entities defined to be placeable
	//enum ModeState {
	//	state_inactive,
	//	state_placing,
	//	state_removing,
	//	Length
	//} m_state = state_inactive;		// state of mode
	//float m_placingDistance = 25.f; // distance of ray tracing on terrain
	//Entity* m_markedEntityToRemove =
	//	nullptr;					// entity pointer in m_entities that is marked for deletion
	//int m_activePlaceableIndex = 0; // index in m_placeable currently selected
	//Keyboard::Keys m_stateSwitchKey = Keyboard::Tab;		  // switch placing mode
	//Keyboard::Keys m_indexIncreaseKey = Keyboard::NumPad2;	  // increase index
	//Keyboard::Keys m_indexDecreaseKey = Keyboard::NumPad1;	  // decrease index
	//Input::MouseButton m_placeKey = Input::MouseButton::LEFT; // place entity
	//Keyboard::Keys m_deleteKey = Keyboard::Back;			  // delete entity
	//Keyboard::Keys m_randomizeKey = Keyboard::NumPad0;		  // randomize values of entity
	//Keyboard::Keys m_saveKey = Keyboard::NumPad9;			  // randomize values of entity

protected:
	vector<SceneEvent> m_events;
	bool m_interactable = false;

	const string m_prePath = "assets/Scenes/";

	float3 m_spawnPoint;
	Player m_player;

	SkyBox m_skybox;
	TerrainManager m_terrains;
	QuadTree<shared_ptr<Entity>> m_entities;
	vector<shared_ptr<SeaEffect>> m_seaEffects;
	vector<shared_ptr<Fruit>> m_fruits;
	vector<shared_ptr<Entity>> m_bowls;
	vector<shared_ptr<ParticleSystem>> m_particleEffects;
	vector<shared_ptr<Animal>> m_animals;

	// -- Functions --
	void pushEvent(SceneEvent _event);

public:
	void pickUpFruit(FruitType fruitType);
	void dropFruit();
	SceneEvent popEvent();

	virtual void restart();
	virtual void update(float dt);

	virtual void draw();

	void operator=(const Scene& other) = delete;

	void clear();
	void loadTEPS(string filename);
	void load(string sceneName);
	Scene();
	virtual ~Scene();
};
