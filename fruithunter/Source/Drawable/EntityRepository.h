#pragma once
#include "Entity.h"
#include "Input.h"

#define AUTOSAVE 0

class EntityRepository {

private:
	const string m_entityPlacementFilePath = "assets/EntityPlacements/";
	const string m_fileEndings = ".teps"; //tep = Terrain Entity Placements

	struct EntityInstance {
		float3 position, scale, rotation;
		bool operator==(const EntityInstance& other) {
			return (
				position == other.position && scale == other.scale && rotation == other.rotation);
		}
		EntityInstance(float3 _position = float3(0,0,0), float3 _scale = float3(1,1,1), float3 _rotation = float3(0,0,0)) {
			position = _position;
			scale = _scale;
			rotation = _rotation;
		}
	};
	struct EntityInstances {
		string meshName;
		
		vector<EntityInstance> instances;
		EntityInstances(string name = "") { meshName = name; }
	};

	//repository stuff
	vector<EntityInstances> m_repository;
	bool m_repositoryLoaded = false;//true if loadPlacement() was called and succesful
	bool m_repositoryChangedSinceLoad = false;
	string m_repositoryFilenameLoadedFrom = "";

	vector<unique_ptr<Entity>> m_entities;// array used to store placed entities for drawing

	//placeable stuff
	vector<unique_ptr<Entity>> m_placeable;//entities defined to be placeable
	bool m_placing = false;
	int m_activePlaceableIndex = 0;
	Keyboard::Keys m_stateSwitchKey = Keyboard::Tab;//switch placing mode
	Keyboard::Keys m_indexIncreaseKey = Keyboard::NumPad2;//increase index
	Keyboard::Keys m_indexDecreaseKey = Keyboard::NumPad1;//decrease index
	Keyboard::Keys m_placeKey = Keyboard::Enter;//place entity
	Keyboard::Keys m_deleteKey = Keyboard::Back;//delete entity
	Keyboard::Keys m_randomizeKey = Keyboard::NumPad0;//randomize values of entity
	Keyboard::Keys m_saveKey = Keyboard::NumPad9;//randomize values of entity

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
	void removeEntity(const Entity* entity);

	EntityInstance getEntityInstance(const Entity* entity) const;
	void setEntityByInstance(Entity* entity, EntityInstance instance);

public:

	void load(string filename);
	void save();

	void addPlaceableEntity(string meshFilename);

	void update(float dt, float3 point, float3 direction);
	void draw();

	EntityRepository(string filename = "");
	~EntityRepository();

};
