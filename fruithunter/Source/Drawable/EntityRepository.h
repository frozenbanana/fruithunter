#pragma once
#include "Entity.h"
#include "Input.h"
#include "QuadTree.h"

class EntityRepository {

private:
	const string m_entityPlacementFilePath = "assets/EntityPlacements/";
	const string m_fileEndings = ".teps"; // tep = Terrain Entity Placements

	// modifiers for collision
	const vector<string> m_treeNames = { "treeMedium1", "treeMedium2", "treeMedium3", "BurnedTree1",
		"BurnedTree2", "BurnedTree3" };
	const vector<string> m_nonCollidables = { "DeadBush", "Grass1", "Grass2", "Grass3", "Grass4" };

	struct EntityInstance {
		float3 position, scale;
		float4x4 matRotation;
		bool operator==(const EntityInstance& other) {
			return (position == other.position && scale == other.scale &&
					matRotation == other.matRotation);
		}
		EntityInstance(float3 _position = float3(0, 0, 0), float3 _scale = float3(1, 1, 1),
			float4x4 _matRotation = XMMatrixIdentity()) {
			position = _position;
			scale = _scale;
			matRotation = _matRotation;
		}
	};
	struct EntityInstances {
		string meshName;

		vector<EntityInstance> instances;
		EntityInstances(string name = "") { meshName = name; }
	};

	// repository stuff
	vector<EntityInstances> m_repository;
	bool m_repositoryLoaded = false;		   // true if loadPlacement() was called and succesful
	bool m_repositoryChangedSinceLoad = false; // true if addEntity/removeEntity was called
	string m_repositoryFilenameLoadedFrom =
		""; // loaded repository, writes to this filename at saving

	vector<unique_ptr<Entity>> m_entities; // array used to store placed entities for drawing
	QuadTree<Entity*> m_quadtree;

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

	//-- Functions --
	void clear();

	void fillEntitiesFromRepository();

	void savePlacements(string filename) const;
	void loadPlacements(string filename);
	void createFile(string filename) const;
	bool fileExists(string filename) const;

	float random(float min, float max) const;
	void randomizeProperties(Entity* entity) const;

	void addEntity(string meshFilename, EntityInstance instance);
	void removeEntity(Entity* entity);

	EntityInstance getEntityInstance(const Entity* entity) const;
	void setEntityByInstance(Entity* entity, EntityInstance instance);

	bool tryTreeCollisionData(Entity* entity);
	bool tryNonCollidable(Entity* entity);
	void assignCollisionData(Entity* entity);

public:
	vector<unique_ptr<Entity>>* getEntities();
	vector<Entity**> getCulledEntitiesByPosition(float3 position);
	void load(string filename);
	void save();

	void addPlaceableEntity(string meshFilename);

	void update(float dt, float3 point, float3 direction);
	void draw();
	void draw_quadtreeFrustumCulling(const vector<FrustumPlane>& planes);
	void draw_quadtreeBBCulling(const CubeBoundingBox& bb);
	void drawShadow();

	EntityRepository(string filename = "");
	~EntityRepository();
};
