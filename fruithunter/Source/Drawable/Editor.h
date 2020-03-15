#pragma once
#include "Scene.h"
class Editor : public Scene {
private:
	const string m_entityPlacementFilePath = "assets/EntityPlacements/";
	const string m_fileEndings = ".teps"; // tep = Terrain Entity Placements

	// modifiers for collision
	const vector<string> m_treeNames = { "treeMedium1", "treeMedium2", "treeMedium3", "BurnedTree1",
		"BurnedTree2", "BurnedTree3" };
	const vector<string> m_nonCollidables = { "DeadBush", "Grass1", "Grass2", "Grass3", "Grass4" };

	// placeable stuff
	vector<unique_ptr<Entity>> m_placeable; // entities defined to be placeable
	enum ModeState {
		state_inactive,
		state_placing,
		state_removing,
		Length
	} m_state = state_inactive;		// state of mode
	float m_placingDistance = 25.f; // distance of ray tracing on terrain
	Entity* m_markedEntityToRemove =
		nullptr;					// entity pointer in m_entities that is marked for deletion
	int m_activePlaceableIndex = 0; // index in m_placeable currently selected
	Keyboard::Keys m_stateSwitchKey = Keyboard::Tab;		  // switch placing mode
	Keyboard::Keys m_indexIncreaseKey = Keyboard::NumPad2;	  // increase index
	Keyboard::Keys m_indexDecreaseKey = Keyboard::NumPad1;	  // decrease index
	Input::MouseButton m_placeKey = Input::MouseButton::LEFT; // place entity
	Keyboard::Keys m_deleteKey = Keyboard::Back;			  // delete entity
	Keyboard::Keys m_randomizeKey = Keyboard::NumPad0;		  // randomize values of entity
	Keyboard::Keys m_saveKey = Keyboard::NumPad9;			  // randomize values of entity

	// -- Functions --
	void randomizeProperties(Entity* entity) const;

	bool tryTreeCollisionData(Entity* entity);
	bool tryNonCollidable(Entity* entity);
	void assignCollisionData(Entity* entity);

	void saveEntities();
	void save();

public:

	void restart();
	void update(float dt);
	void draw();

	Editor();
	~Editor();
};
