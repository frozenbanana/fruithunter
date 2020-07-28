#include "Scene.h"
#include "PathFindingThread.h"
#include "SaveManager.h"

void Scene::clear() {
	// heightmap
	m_terrains.clear();
	// sea
	m_seaEffects.clear();
	// entities
	m_repository.clear();
	// arrows
	m_arrows.clear();
	// particleSystems
	m_particleSystems.clear();
	// player spawn
	m_playerStartPos = float3(0.);
	// utility
	m_utility = SceneAbstactContent::SceneUtilityInfo();
	// spawn locations
	for (size_t i = 0; i < NR_OF_FRUITS; i++)
		m_fruitSpawnAreas[i].clear();

	// spawn fruit
	m_fruits.clear();
	// timer
	m_timer.reset();

}

void Scene::saveWin() {
	int level = m_utility.levelIndex;
	if (level != -1) {
		size_t time = round(m_timer.getTimePassed());
		TimeTargets grade = TimeTargets::BRONZE;
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
			if (time < m_utility.timeTargets[i]) {
				grade = (TimeTargets)i;
				break;
			}
		}
		SaveManager::getInstance()->setLevelCompletion(level, time, grade);
	}
}

void Scene::resetPlayer() {
	m_player = make_shared<Player>();
	m_player->setPosition(m_playerStartPos);
}

Scene::Scene(string filename) { 
	// Initiate entity repos
	m_repository.addPlaceableEntity("treeMedium1");
	m_repository.addPlaceableEntity("treeMedium2");
	m_repository.addPlaceableEntity("treeMedium3");
	m_repository.addPlaceableEntity("stone1");
	m_repository.addPlaceableEntity("stone2");
	m_repository.addPlaceableEntity("stone3");
	m_repository.addPlaceableEntity("bush1");
	m_repository.addPlaceableEntity("bush2");
	m_repository.addPlaceableEntity("DeadBush");
	m_repository.addPlaceableEntity("BurnedTree1");
	m_repository.addPlaceableEntity("BurnedTree2");
	m_repository.addPlaceableEntity("BurnedTree3");
	m_repository.addPlaceableEntity("Cactus_tall");
	m_repository.addPlaceableEntity("Cactus_small");
	m_repository.addPlaceableEntity("Grass1");
	m_repository.addPlaceableEntity("Grass2");
	m_repository.addPlaceableEntity("Grass3");
	m_repository.addPlaceableEntity("Grass4");
	m_repository.addPlaceableEntity("RopeBridgeFloor");
	m_repository.addPlaceableEntity("RopeBridgeRailing1");
	m_repository.addPlaceableEntity("RopeBridgeRailing2");

	load(filename);
}

Scene::~Scene() { PathFindingThread::getInstance()->pause(); }

void Scene::pickUpFruit(FruitType fruitType) { m_utility.gathered[fruitType]++; }

void Scene::dropFruit(FruitType fruitType) {
	 Input* ip = Input::getInstance();
	 auto pft = PathFindingThread::getInstance();

	 if (m_utility.gathered[fruitType] > 0) {
		 //reduce inventory
		 if (!DEBUG)
			 m_utility.gathered[fruitType]--;
		 //spawn fruit
		 shared_ptr<Fruit> fruit = Fruit::createFruitFromType(fruitType);
		 fruit->setPosition(m_player->getPosition() + float3(0.0f, 1.5f, 0.0f));
		 fruit->release(m_player->getForward());
		 PathFindingThread::lock();
		 m_fruits.push_back(fruit);
		 PathFindingThread::unlock();
	 }
}

void Scene::load(string folder) { 
	if (folder != "") {
		// clear all data
		clear();

		// load content
		content.load(folder);

		// heightmap
		for (size_t i = 0; i < content.m_heightmapAreas.size(); i++) {
			SceneAbstactContent::HeightmapContent* c = &content.m_heightmapAreas[i];
			string textures[4] = { c->texture0, c->texture1, c->texture2, c->texture3 };
			m_terrains.add(c->position, c->scale, c->heightmapName, textures, c->subSize,
				c->division, c->wind, (AreaTag)c->areaTag);
		}
		// sea
		m_seaEffects.resize(content.m_seaAreas.size());
		for (size_t i = 0; i < content.m_seaAreas.size(); i++) {
			SceneAbstactContent::SeaContent* c = &content.m_seaAreas[i];
			m_seaEffects[i].initilize(
				(SeaEffect::SeaEffectTypes)c->type, c->tiles, c->grids, c->position, c->scale);
		}
		// entities
		m_repository.load(content.m_entityStorageFilename);
		// particleSystems
		m_particleSystems.resize(content.m_particleSystemContents.size());
		for (size_t i = 0; i < content.m_particleSystemContents.size(); i++) {
			SceneAbstactContent::ParticleSystemContent* c = &content.m_particleSystemContents[i];
			m_particleSystems[i].load((ParticleSystem::PARTICLE_TYPE)c->type);
			m_particleSystems[i].setPosition(c->position);
		}
		// player spawn
		m_playerStartPos = content.m_playerStartPos;
		// utility
		m_utility = content.utility;
		// spawn locations
		for (size_t i = 0; i < content.m_heightmapAreas.size(); i++)
			for (size_t j = 0; j < NR_OF_FRUITS; j++)
				if (content.m_heightmapAreas[i].canSpawnFruit[j])
					m_fruitSpawnAreas[j].push_back(i);

		// spawn fruit
		for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++) {
			for (size_t iCount = 0; iCount < m_utility.nrOfFruits[iFruit]; iCount++) {
				if (m_fruitSpawnAreas[iFruit].size() > 0) {
					int index =
						m_fruitSpawnAreas[iFruit][rand() % m_fruitSpawnAreas[iFruit].size()];
					float3 spawn = m_terrains.getSpawnpoint(index);
					switch (iFruit) {
					case FruitType::APPLE:
						m_fruits.push_back(make_shared<Apple>(spawn));
						break;
					case FruitType::BANANA:
						m_fruits.push_back(make_shared<Banana>(spawn));
						break;
					case FruitType::MELON:
						m_fruits.push_back(make_shared<Melon>(spawn));
						break;
					case FruitType::DRAGON:
						m_fruits.push_back(make_shared<DragonFruit>(spawn));
						break;
					}
				}
			}
		}

		// player
		resetPlayer();
	}
}

void Scene::save(string folder) {
	
}

bool Scene::handleWin() {

	// check if winCondition are met
	bool hasWon = true;
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (m_utility.gathered[i] < m_utility.winCondition[i]) {
			hasWon = false;
			break;
		}
	}
	if (hasWon) {
		//save win
		int level = m_utility.levelIndex;
		if (level != -1) {
			size_t time = round(m_timer.getTimePassed());
			TimeTargets grade = TimeTargets::BRONZE;
			for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
				if (time < m_utility.timeTargets[i]) {
					grade = (TimeTargets)i;
					break;
				}
			}
			SaveManager::getInstance()->setLevelCompletion(level, time, grade);
		}
	}
	return hasWon;
}

void SceneAbstactContent::load(string folder) {
	this->folder = folder;
	// heightmap content
	m_heightmapAreas.clear();
	int index = 0;
	string path = prePath + folder + "/heightmap_";
	while (FileSyncer::fileExists(path + to_string(index) + ".txt")) {
		m_heightmapAreas.resize(m_heightmapAreas.size() + 1);
		FileSyncer* file = &m_heightmapAreas.back().file;
		file->connect("bla");//makes file valid
		file->bind("tag:i", &m_heightmapAreas.back().areaTag);
		file->bind("heightmapName:s", &m_heightmapAreas.back().heightmapName);
		file->bind("position:v3", &m_heightmapAreas.back().position);
		file->bind("scale:v3", &m_heightmapAreas.back().scale);
		file->bind("subSizeX:i", &m_heightmapAreas.back().subSize.x);
		file->bind("subSizeY:i", &m_heightmapAreas.back().subSize.y);
		file->bind("divisionX:i", &m_heightmapAreas.back().division.x);
		file->bind("divisionY:i", &m_heightmapAreas.back().division.y);
		file->bind("tex0:s", &m_heightmapAreas.back().texture0);
		file->bind("tex1:s", &m_heightmapAreas.back().texture1);
		file->bind("tex2:s", &m_heightmapAreas.back().texture2);
		file->bind("tex3:s", &m_heightmapAreas.back().texture3);
		file->bind("wind:v3", &m_heightmapAreas.back().wind);
		file->bind("canSpawn_Apple:i", &m_heightmapAreas.back().canSpawnFruit[0]);
		file->bind("canSpawn_Banana:i", &m_heightmapAreas.back().canSpawnFruit[1]);
		file->bind("canSpawn_Melon:i", &m_heightmapAreas.back().canSpawnFruit[2]);
		file->bind("canSpawn_Dragon:i", &m_heightmapAreas.back().canSpawnFruit[3]);
		file->readFile(path + to_string(index) + ".txt");
		index++;
	}

	// sea content
	m_seaAreas.clear();
	index = 0;
	path = prePath + folder + "/sea_";
	while (FileSyncer::fileExists(path + to_string(index)+".txt")) {
		m_seaAreas.resize(m_seaAreas.size() + 1);
		FileSyncer* file = &m_seaAreas.back().file;
		file->connect("bla");//makes file valid
		file->bind("type:i", &m_seaAreas.back().type);
		file->bind("position:v3", &m_seaAreas.back().position);
		file->bind("scale:v3", &m_seaAreas.back().scale);
		file->bind("tilesX:i", &m_seaAreas.back().tiles.x);
		file->bind("tilesY:i", &m_seaAreas.back().tiles.y);
		file->bind("gridsX:i", &m_seaAreas.back().grids.x);
		file->bind("gridsY:i", &m_seaAreas.back().grids.y);
		file->readFile(path + to_string(index) + ".txt");
		index++;
	}

	// particlesystem content
	m_particleSystemContents.clear();
	index = 0;
	path = prePath + folder + "/particleSystem_";
	while (FileSyncer::fileExists(path + to_string(index) + ".txt")) {
		m_particleSystemContents.resize(m_particleSystemContents.size() + 1);
		FileSyncer* file = &m_particleSystemContents.back().file;
		file->connect("bla"); // makes file valid
		file->bind("type:i", &m_particleSystemContents.back().type);
		file->bind("position:v3", &m_particleSystemContents.back().position);
		file->readFile(path + to_string(index) + ".txt");
		index++;
	}

	file.clear();
	file.connect("bla");
	file.bind("entityStorageFilename:s", &m_entityStorageFilename);
	file.bind("level:i", &utility.levelIndex);
	file.bind("spawn:v3", &m_playerStartPos);
	file.bind("nrOfFruits_Apple:i", &utility.nrOfFruits[0]);
	file.bind("nrOfFruits_Banana:i", &utility.nrOfFruits[1]);
	file.bind("nrOfFruits_Melon:i", &utility.nrOfFruits[2]);
	file.bind("nrOfFruits_Dragon:i", &utility.nrOfFruits[3]);
	file.bind("winCondition_Apple:i", &utility.winCondition[0]);
	file.bind("winCondition_Banana:i", &utility.winCondition[1]);
	file.bind("winCondition_Melon:i", &utility.winCondition[2]);
	file.bind("winCondition_Dragon:i", &utility.winCondition[3]);
	file.bind("timeTarget_Gold:i", &utility.timeTargets[0]);
	file.bind("timeTarget_Silver:i", &utility.timeTargets[1]);
	file.bind("timeTarget_Bronze:i", &utility.timeTargets[2]);

	  /*Goal is missing*/
	file.readFile(prePath + folder + "/other.txt");

}

SceneAbstactContent::SceneAbstactContent() { }
