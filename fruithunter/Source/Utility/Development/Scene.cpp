#include "Scene.h"
#include "PathFindingThread.h"
#include "SaveManager.h"
#include "fileSystemHelper.h"

void Scene::clear() {
	PathFindingThread::lock();
	// heightmap
	m_terrains.clear();
	// sea
	m_seaEffects.clear();
	// entities
	m_entities.clear();
	// arrows
	m_arrows.clear();
	// particleSystems
	m_particleSystems.clear();
	// animals
	m_animals.clear();
	// spawn fruit
	m_fruits.clear();
	//collection points
	m_collectionPoint.clear();

	// utility
	m_utility = SceneAbstactContent::SceneUtilityInfo();
	// spawn locations
	m_fruitSpawns.clear();
	// Gathered fruit
	for (size_t i = 0; i < NR_OF_FRUITS; i++)
		m_gatheredFruits[i] = 0;

	// timer
	m_timer.reset();

	m_sceneName = "";
	m_loaded = false;

	PathFindingThread::unlock();
}

void Scene::saveWin() {
	if (m_sceneName != "") {
		size_t time = (size_t)round(m_timer.getTimePassed());
		TimeTargets grade = TimeTargets::NR_OF_TIME_TARGETS;
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
			if (time <= m_utility.timeTargets[i]) {
				grade = (TimeTargets)i;
				break;
			}
		}
		SaveManager::setProgress(m_sceneName, time, grade);
	}
}

Scene::Scene(string filename) { 
	load(filename);
}

Scene::~Scene() { PathFindingThread::getInstance()->pause(); }

size_t Scene::find_parentIndex(Fragment* fragment) { 
	switch (fragment->getType()) {
	case Fragment::Type::animal:
		for (size_t i = 0; i < m_animals.size(); i++) {
			if (m_animals[i]->getID() == fragment->getID()) {
				return i;
			}
		}
		break;
	case Fragment::Type::entity:
		for (size_t i = 0; i < m_entities.size(); i++) {
			if (m_entities[i]->getID() == fragment->getID())
				return i;
		}
		break;
	case Fragment::Type::particleSystem:
		for (size_t i = 0; i < m_particleSystems.size(); i++) {
			if (m_particleSystems[i].getID() == fragment->getID()) {
				return i;
			}
		}
		break;
	case Fragment::Type::sea:
		for (size_t i = 0; i < m_seaEffects.size(); i++) {
			if (m_seaEffects[i]->getID() == fragment->getID()) {
				return i;
			}
		}
		break;
	case Fragment::Type::terrain:
		for (size_t i = 0; i < m_terrains.length(); i++) {
			if (m_terrains.getTerrainFromIndex(i)->getID() == fragment->getID()) {
				return i;
			}
		}
		break;
	}
	return -1;
}

bool Scene::remove_fragment(Fragment* fragment) { 
	switch (fragment->getType()) {
	case Fragment::Type::animal:
		for (size_t i = 0; i < m_animals.size(); i++) {
			if (m_animals[i]->getID() == fragment->getID()) {
				m_animals.erase(m_animals.begin() + i);
				return true;
			}
		}
		break;
	case Fragment::Type::entity:
		for (size_t i = 0; i < m_entities.size(); i++) {
			if (m_entities[i]->getID() == fragment->getID()) {
				m_entities.remove(i);
				return true;
			}
		}
		break;
	case Fragment::Type::particleSystem:
		for (size_t i = 0; i < m_particleSystems.size(); i++) {
			if (m_particleSystems[i].getID() == fragment->getID()) {
				m_particleSystems.erase(m_particleSystems.begin() + i);
				return true;
			}
		}
		break;
	case Fragment::Type::sea:
		for (size_t i = 0; i < m_seaEffects.size(); i++) {
			if (m_seaEffects[i]->getID() == fragment->getID()) {
				m_seaEffects.erase(m_seaEffects.begin() + i);
				return true;
			}
		}
		break;
	case Fragment::Type::terrain:
		for (size_t i = 0; i < m_terrains.length(); i++) {
			if (m_terrains.getTerrainFromIndex(i)->getID() == fragment->getID()) {
				m_terrains.remove(i);
				return true;
			}
		}
		break;
	}
	return false;
	
}

void Scene::updated_fragment(Fragment* fragment) {
	switch (fragment->getType()) {
	case Fragment::Type::animal:
		break;
	case Fragment::Type::entity:
		for (size_t i = 0; i < m_entities.size(); i++) {
			if (m_entities[i]->getID() == fragment->getID()) {
				shared_ptr<Entity> ptr = m_entities[i];
				m_entities.remove(i);
				m_entities.add(ptr->getLocalBoundingBoxPosition(), ptr->getLocalBoundingBoxSize(),
					ptr->getMatrix(), ptr);
			}
		}
		break;
	case Fragment::Type::particleSystem:
		break;
	case Fragment::Type::sea:
		break;
	case Fragment::Type::terrain:
		break;
	}
}

void Scene::pickUpFruit(FruitType fruitType) { m_gatheredFruits[fruitType]++; }

void Scene::dropFruit(FruitType fruitType) {
	 Input* ip = Input::getInstance();
	 auto pft = PathFindingThread::getInstance();

	 if (m_gatheredFruits[fruitType] > 0) {
		 //reduce inventory
		 if (!DEBUG)
			 m_gatheredFruits[fruitType]--;
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
		clear();// clear all data

		m_loaded = true;
		m_sceneName = folder;

		PathFindingThread::lock();
		// load content
		SceneAbstactContent content;
		content.load_raw(folder);

		// heightmap
		for (size_t i = 0; i < content.m_heightmapAreas.size(); i++) {
			SceneAbstactContent::HeightmapContent* c = &content.m_heightmapAreas[i];
			m_terrains.add(c->position, c->scale, c->heightmapName, c->textures, c->subSize,
				c->division, c->wind, (AreaTag)c->areaTag);
		}
		// sea
		m_seaEffects.resize(content.m_seaAreas.size());
		for (size_t i = 0; i < content.m_seaAreas.size(); i++) {
			SceneAbstactContent::SeaContent* c = &content.m_seaAreas[i];
			m_seaEffects[i] = make_shared<SeaEffect>();
			m_seaEffects[i]->initilize(
				(SeaEffect::SeaEffectTypes)c->type, c->tiles, c->grids, c->position, c->scale);
		}
		// entities
		size_t entityCount = 0;
		for (size_t i = 0; i < content.m_entities.size(); i++)
			entityCount += content.m_entities[i].instances.size();
		m_entities.reserve(entityCount);
		for (size_t i = 0; i < content.m_entities.size(); i++) {
			string model = content.m_entities[i].model;
			for (size_t j = 0; j < content.m_entities[i].instances.size(); j++) {
				SceneAbstactContent::GroupInstance::Instance* instance =
					&content.m_entities[i].instances[j];
				shared_ptr<Entity> e =
					make_shared<Entity>(model, instance->position, instance->scale);
				e->setRotation(instance->rotation);
				e->setCollidable(instance->collidable);
				m_entities.add(e->getLocalBoundingBoxPosition(), e->getLocalBoundingBoxSize(),
					e->getMatrix(), e);
			}
		}
		// particle Systems
		m_particleSystems.resize(content.m_particleSystemContents.size());
		for (size_t i = 0; i < content.m_particleSystemContents.size(); i++) {
			SceneAbstactContent::ParticleSystemContent* c = &content.m_particleSystemContents[i];
			m_particleSystems[i].load((ParticleSystem::Type)c->type, c->emitRate, c->capacity);
			m_particleSystems[i].setPosition(c->position);
			m_particleSystems[i].setScale(c->size);
			m_particleSystems[i].affectedByWindState(c->affectedByWind);
		}
		// animals
		m_animals.resize(content.m_animals.size());
		for (size_t i = 0; i < m_animals.size(); i++) {
			SceneAbstactContent::AnimalContent* c = &content.m_animals[i];
			m_animals[i] = make_shared<Animal>(c->position, c->sleepPosition, (Animal::Type)c->type,
				(FruitType)c->fruitType, c->fruitCount, c->rotationY);
		}
		// utility
		m_utility = content.m_utility;
		// spawn locations
		m_fruitSpawns.resize(content.m_heightmapAreas.size());
		for (size_t i = 0; i < content.m_heightmapAreas.size(); i++) {
			for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++) {
				m_fruitSpawns[i].quantity[iFruit] +=
					content.m_heightmapAreas[i].spawnFruits[iFruit];
			}
		}
		PathFindingThread::unlock();

		reset();
	}
}

void Scene::save() {
	if (m_sceneName != "") {
		SceneAbstactContent content;

		// terrains
		content.m_heightmapAreas.resize(m_terrains.length());
		for (size_t i = 0; i < m_terrains.length(); i++) {
			SceneAbstactContent::HeightmapContent* hc = &content.m_heightmapAreas[i];
			Terrain* t = m_terrains.getTerrainFromIndex(i).get();
			hc->areaTag = (int)t->getTag();
			hc->heightmapName = t->getLoadedHeightmapFilename();
			hc->position = t->getPosition();
			hc->rotation = t->getRotation();
			hc->scale = t->getScale();
			hc->subSize = t->getSubSize();
			hc->division = t->getSplits();
			t->getTextures(hc->textures);
			hc->wind = t->getWindStatic();
			for (size_t j = 0; j < NR_OF_FRUITS; j++) {
				hc->spawnFruits[j] = m_fruitSpawns[i].quantity[j];
			}
		}

		// seas
		content.m_seaAreas.resize(m_seaEffects.size());
		for (size_t i = 0; i < m_seaEffects.size(); i++) {
			SceneAbstactContent::SeaContent* c = &content.m_seaAreas[i];
			SeaEffect* sea = m_seaEffects[i].get();
			c->type = (int)sea->getType();
			c->position = sea->getPosition();
			c->rotation = sea->getRotation();
			c->scale = sea->getScale();
			c->tiles = sea->getTileSize();
			c->grids = sea->getGridSize();
		}

		// particlesystems
		content.m_particleSystemContents.resize(m_particleSystems.size());
		for (size_t i = 0; i < m_particleSystems.size(); i++) {
			SceneAbstactContent::ParticleSystemContent* c = &content.m_particleSystemContents[i];
			ParticleSystem* ps = &m_particleSystems[i];
			c->type = (int)ps->getType();
			c->position = ps->getPosition();
			c->size = ps->getScale();
			c->affectedByWind = ps->isAffectedByWind();
			c->emitRate = ps->getEmitRate();
			c->capacity = ps->getCapacity();
		}

		// entities
		vector<SceneAbstactContent::GroupInstance>* group = &content.m_entities;
		for (size_t i = 0; i < m_entities.size(); i++) {
			string model = m_entities[i]->getModelName();
			SceneAbstactContent::GroupInstance::Instance instance(m_entities[i]->getPosition(),
				m_entities[i]->getRotation(), m_entities[i]->getScale(), m_entities[i]->getIsCollidable());
			bool found = false;
			for (size_t j = 0; j < group->size(); j++) {
				if (group->at(j).model == model) {
					group->at(j).instances.push_back(instance);
					found = true;
					break;
				}
			}
			if (!found) {
				group->push_back(SceneAbstactContent::GroupInstance(model));
				group->back().instances.push_back(instance);
			}
		}

		// animals
		content.m_animals.resize(m_animals.size());
		for (size_t i = 0; i < m_animals.size(); i++) {
			SceneAbstactContent::AnimalContent *c = &content.m_animals[i];
			Animal* a = m_animals[i].get();
			c->fruitCount = a->getRequiredFruitCount();
			c->fruitType = a->getfruitType();
			c->position = a->getPosition();
			c->rotationY = a->getRotation().y;
			c->sleepPosition = a->getSleepPosition();
			c->type = a->getType();
		}

		// level utility
		content.m_utility = m_utility;

		content.save_raw(m_sceneName);

		ErrorLogger::log("Saved scene: " + m_sceneName);
	}
}

void Scene::reset() { 
	PathFindingThread::lock();

	//arrows
	m_arrows.clear(); 
	//animals
	for (size_t i = 0; i < m_animals.size(); i++)
		m_animals[i]->reset();
	//fruits
	m_fruits.clear();
	size_t total = 0;
	for (size_t i = 0; i < m_fruitSpawns.size(); i++)
		for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++)
			total = m_fruitSpawns[i].quantity[iFruit];
	m_fruits.reserve(total);
	for (size_t i = 0; i < m_fruitSpawns.size(); i++) {
		for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++) {
			size_t count = m_fruitSpawns[i].quantity[iFruit];
			for (size_t j = 0; j < count; j++) {
				float3 spawn = m_terrains.getSpawnpoint(i);
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
	//player
	m_player = make_shared<Player>();
	m_player->setPosition(m_utility.startSpawn);
	//collectionPoints
	m_collectionPoint.clear();
	//gathered fruit
	for (size_t i = 0; i < NR_OF_FRUITS; i++)
		m_gatheredFruits[i] = 0;
	// timer
	m_timer.reset();

	PathFindingThread::unlock();
}

bool Scene::handleWin() {

	// check if winCondition are met
	bool hasWon = true;
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (m_gatheredFruits[i] < m_utility.winCondition[i]) {
			hasWon = false;
			break;
		}
	}
	if (hasWon) {
		saveWin();
	}
	return hasWon;
}

float Scene::getDeltaTime() {
	float dt = m_timer.getDt();
	if (m_player->inHuntermode())
		dt *= 0.1;
	return dt;
}

float Scene::getDeltaTime_skipSlow() { return m_timer.getDt(); }

void SceneAbstactContent::fileInput_string(fstream& file, string str) { 
	size_t length = str.length();
	fileInput_ulong(file, length);
	file.write(str.c_str(),length); 
}

string SceneAbstactContent::fileOutput_string(fstream& file) { 
	size_t length = fileOutput_ulong(file);
	char* text = new char[length];
	file.read(text, length);
	string ret;
	ret.insert(0,text, length);
	delete[] text;
	return ret;
}

void SceneAbstactContent::fileInput_ulong(fstream& file, size_t v) {
	file.write((char*)&v, sizeof(size_t));
}

size_t SceneAbstactContent::fileOutput_ulong(fstream& file) { 
	size_t ret;
	file.read((char*)&ret, sizeof(size_t));
	return ret;
}

bool SceneAbstactContent::load_raw(string folder) {
	string path = path_scenes + folder;
	fstream file;
	file.open(path+"/scene.data", ios::in | ios::binary);
	if (file.is_open()) {
		// terrain
		size_t size = fileOutput_ulong(file);
		m_heightmapAreas.resize(size);
		for (size_t i = 0; i < size; i++) {
			file.read((char*)&m_heightmapAreas[i].areaTag, sizeof(int));//area tag
			m_heightmapAreas[i].heightmapName = fileOutput_string(file);//heightmap name
			file.read((char*)&m_heightmapAreas[i].position, sizeof(float3));//position
			file.read((char*)&m_heightmapAreas[i].rotation, sizeof(float3));//rotation
			file.read((char*)&m_heightmapAreas[i].scale, sizeof(float3));//scale
			file.read((char*)&m_heightmapAreas[i].subSize, sizeof(XMINT2));//subsize
			file.read((char*)&m_heightmapAreas[i].division, sizeof(XMINT2));//divisions
			for (size_t itex = 0; itex < 4; itex++) 
				m_heightmapAreas[i].textures[itex] = fileOutput_string(file);//textures
			file.read((char*)&m_heightmapAreas[i].wind, sizeof(float3));//wind
			for (size_t ifruit = 0; ifruit < NR_OF_FRUITS; ifruit++) 
				file.read((char*)&m_heightmapAreas[i].spawnFruits[ifruit], sizeof(int));//fruit spawns
		}
		// seas
		size = fileOutput_ulong(file);
		m_seaAreas.resize(size);
		for (size_t i = 0; i < size; i++)
			file.read((char*)&m_seaAreas[i], sizeof(SeaContent));
		// particlesystems
		size = fileOutput_ulong(file);
		m_particleSystemContents.resize(size);
		for (size_t i = 0; i < size; i++)
			file.read((char*)&m_particleSystemContents[i], sizeof(ParticleSystemContent));
		// entities
		size = fileOutput_ulong(file);
		m_entities.resize(size);
		for (size_t i = 0; i < size; i++) {
			m_entities[i].model = fileOutput_string(file);
			size_t subSize = fileOutput_ulong(file);
			m_entities[i].instances.resize(subSize);
			file.read(
				(char*)m_entities[i].instances.data(), sizeof(GroupInstance::Instance) * subSize);
		}
		//animals
		size = fileOutput_ulong(file);
		m_animals.resize(size);
		for (size_t i = 0; i < size; i++) {
			file.read((char*)&m_animals[i], sizeof(AnimalContent));
		}
		// utility
		file.read((char*)&m_utility, sizeof(SceneUtilityInfo));

		file.close();
		return true;
	}
	return false;
}

bool SceneAbstactContent::save_raw(string folder) { 
	string path = path_scenes + folder;
	create_directory(path);
	fstream file;
	file.open(path+"/scene.data", ios::out | ios::binary);
	if (file.is_open()) {
		//terrain
		fileInput_ulong(file, m_heightmapAreas.size());
		for (size_t i = 0; i < m_heightmapAreas.size(); i++) {
			file.write((char*)&m_heightmapAreas[i].areaTag, sizeof(int));	 // area tag
			fileInput_string(file, m_heightmapAreas[i].heightmapName);		  // heightmap name
			file.write((char*)&m_heightmapAreas[i].position, sizeof(float3)); // position
			file.write((char*)&m_heightmapAreas[i].rotation, sizeof(float3)); // rotation
			file.write((char*)&m_heightmapAreas[i].scale, sizeof(float3));	  // scale
			file.write((char*)&m_heightmapAreas[i].subSize, sizeof(XMINT2));  // subsize
			file.write((char*)&m_heightmapAreas[i].division, sizeof(XMINT2)); // divisions
			for (size_t itex = 0; itex < 4; itex++)
				fileInput_string(file, m_heightmapAreas[i].textures[itex]); // textures
			file.write((char*)&m_heightmapAreas[i].wind, sizeof(float3));	  // wind
			for (size_t ifruit = 0; ifruit < NR_OF_FRUITS; ifruit++)
				file.write(
					(char*)&m_heightmapAreas[i].spawnFruits[ifruit], sizeof(int)); // fruit spawns
		}
		//seas
		fileInput_ulong(file, m_seaAreas.size());
		for (size_t i = 0; i < m_seaAreas.size(); i++)
			file.write((char*)&m_seaAreas[i], sizeof(SeaContent));
		//particlesystems
		fileInput_ulong(file, m_particleSystemContents.size());
		for (size_t i = 0; i < m_particleSystemContents.size(); i++)
			file.write((char*)&m_particleSystemContents[i], sizeof(ParticleSystemContent));
		//entities
		fileInput_ulong(file, m_entities.size());
		for (size_t i = 0; i < m_entities.size(); i++) {
			fileInput_string(file, m_entities[i].model);
			fileInput_ulong(file, m_entities[i].instances.size());
			file.write((char*)m_entities[i].instances.data(),
				sizeof(GroupInstance::Instance) * m_entities[i].instances.size());
		}
		//animals
		fileInput_ulong(file, m_animals.size());
		for (size_t i = 0; i < m_animals.size(); i++) {
			file.write((char*)&m_animals[i], sizeof(AnimalContent));
		}
		//utility
		file.write((char*)&m_utility, sizeof(SceneUtilityInfo));

		file.close();
		return true;
	}
	return false;
}

SceneAbstactContent::SceneAbstactContent() { }
