#include "Scene.h"
#include "PathFindingThread.h"
#include "SaveManager.h"
#include "fileSystemHelper.h"
#include "AudioController.h"

void Scene::clear() {
	PathFindingThread::lock();
	// skybox
	m_skyBox.reset();
	// heightmap
	m_terrains.clear();
	// sea
	m_seaEffects.clear();
	// entities
	m_entities.clear();
	// arrows
	m_arrows.clear();
	m_arrowParticles.clear();
	// particleSystems
	m_particleSystems.clear();
	// spawn fruit
	m_fruits.clear();
	//collection points
	m_collectionPoint.clear();

	// utility
	m_utility = SceneAbstactContent::SceneUtilityInfo();
	// Gathered fruit
	for (size_t i = 0; i < NR_OF_FRUITS; i++)
		m_gatheredFruits[i] = 0;

	// timer
	m_timer.reset();
	// active terrain
	m_activeTerrain_tag = AreaTag::Plains;
	m_activeTerrain_soundID = 0;
	// music
	AudioController::getInstance()->stop(m_activeTerrain_soundID);

	m_sceneName = "";
	m_loaded = false;

	PathFindingThread::unlock();
}

TimeTargets Scene::getTimeTargetGrade(size_t timeMs, size_t timeTargets[NR_OF_TIME_TARGETS]) {
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		if (timeMs <= timeTargets[i])
			return (TimeTargets)i;
	}
	return NR_OF_TIME_TARGETS;
}

void Scene::saveWin() {
	if (m_sceneName != "") {
		size_t time = m_timer.getTimePassedAsMilliseconds();
		SaveManager::getInstance()->setLevelProgress(m_sceneName, time);
	}
}

Scene::Scene(string filename) { 
	load(filename);
}

Scene::~Scene() { PathFindingThread::getInstance()->pause(); }

size_t Scene::find_parentIndex(Fragment* fragment) { 
	switch (fragment->getType()) {
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

void Scene::addArrow(shared_ptr<Arrow> arrow) { 
	if (arrow.get() != nullptr) {
		m_arrows.push_back(arrow);
		if (m_arrows.size() > 10)
			m_arrows.erase(m_arrows.begin());
	}
}

SoundID Scene::playMusicByAreaTag(AreaTag tag) {
	SoundID id = 0;
	switch (tag) {
	case Forest:
		id = AudioController::getInstance()->play(
			"mystic_forest", AudioController::SoundType::Music, true);
		break;
	case Plains:
		id = AudioController::getInstance()->play(
			"tuff_tuff_grazz", AudioController::SoundType::Music, true);
		break;
	case Desert:
		id = AudioController::getInstance()->play(
			"pyramid_cactus", AudioController::SoundType::Music, true);
		break;
	case Volcano:
		id = AudioController::getInstance()->play(
			"crumbling_halls", AudioController::SoundType::Music, true);
		break;
	}
	return id;
}

void Scene::update_activeTerrain(AreaTag tag, bool playMusic) {
	AudioController* ac = AudioController::getInstance();
	if (tag != m_activeTerrain_tag) {
		// handle skybox
		m_skyBox.switchLight(tag);
		// handle music
		if (playMusic) {
			const float fadeInTime = 1;
			AudioController::getInstance()->fadeOut(m_activeTerrain_soundID, fadeInTime);
			m_activeTerrain_soundID = playMusicByAreaTag(tag);
			AudioController::getInstance()->fadeIn(m_activeTerrain_soundID, fadeInTime);
		}
		// update tag
		m_activeTerrain_tag = tag;
	}
	else if (!ac->isListed(m_activeTerrain_soundID))
		if (playMusic)
			m_activeTerrain_soundID = playMusicByAreaTag(tag);
}

void Scene::load(string folder) {
	if (folder != "") {
		clear(); // clear all data

		m_loaded = true;
		m_sceneName = folder;

		PathFindingThread::lock();
		// load content
		SceneAbstactContent content;
		content.load_raw(folder);

		// Leaderboard
		m_leaderboardName = content.m_leaderboardName;

		// heightmap
		/*
		* Dynamic file reading. Caused problem of loading terrains even if deleted them in editor (their file wasnt deleted)
		*/
		//vector<string> files;
		//read_directory("assets/Scenes/" + m_sceneName, files);
		//files = vector<string>(files.begin() + 2, files.end());
		//for (size_t i = 0; i < files.size(); i++) {
		//	size_t offset = files[i].find('.', 0);
		//	if (offset != string::npos) {
		//		offset++; // skip '.'
		//		string ending = files[i].substr(offset, files[i].length() - offset);
		//		if (ending == "env") {
		//			shared_ptr<Environment> env = make_shared<Environment>();
		//			env->loadFromBinFile("assets/Scenes/" + m_sceneName + "/" + files[i]);
		//			m_terrains.add(env);
		//		}
		//	}
		//}
		/*
		* Simple system for reading files. Only reads files listed in content variable.
		*/
		vector<string> files = content.m_terrainFiles;
		for (size_t i = 0; i < files.size(); i++) {
			shared_ptr<Environment> env = make_shared<Environment>();
			env->loadFromBinFile("assets/Scenes/" + m_sceneName + "/" + files[i]);
			m_terrains.add(env);
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
		// utility
		m_utility = content.m_utility;

		PathFindingThread::unlock();

		reset();
	}
}

void Scene::save() {
	if (m_sceneName != "") {
		SceneAbstactContent content;

		// Leaderboard
		content.m_leaderboardName = m_leaderboardName;

		// terrains
		content.m_terrainFiles.resize(m_terrains.length());
		for (size_t i = 0; i < m_terrains.length(); i++) {
			string filename = "terrain" + to_string(i) + ".env";
			m_terrains.getTerrainFromIndex(i)->storeToBinFile(
				"assets/Scenes/" + m_sceneName + "/" + filename);
			content.m_terrainFiles[i] = filename;
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

		// level utility
		content.m_utility = m_utility;

		content.save_raw(m_sceneName);

		ErrorLogger::log("Saved scene: " + m_sceneName);
	}
}

void Scene::reset() { 
	PathFindingThread::lock();

	//skybox
	m_skyBox.reset();
	//arrows
	m_arrows.clear(); 
	m_arrowParticles.clear();
	//fruits
	m_fruits.clear();
	size_t total = 0;
	for (size_t i = 0; i < m_terrains.length(); i++)
		for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++)
			total = m_terrains.getTerrainFromIndex(i)->getFruitCount((FruitType)iFruit);
	m_fruits.reserve(total);
	for (size_t i = 0; i < m_terrains.length(); i++) {
		for (size_t iFruit = 0; iFruit < NR_OF_FRUITS; iFruit++) {
			size_t count = m_terrains.getTerrainFromIndex(i)->getFruitCount((FruitType)iFruit);
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
				m_fruits.back()->bindToEnvironment(m_terrains.getTerrainFromIndex(i).get());
			}
		}
	}
	//player
	m_player = make_shared<Player>();
	m_player->setPosition(m_utility.startSpawn);
	// camera
	m_camera.setEye(m_utility.startSpawn);
	//collectionPoints
	m_collectionPoint.clear();
	//gathered fruit
	for (size_t i = 0; i < NR_OF_FRUITS; i++)
		m_gatheredFruits[i] = 0;
	// timer
	m_timer.reset();

	// active terrain
	m_activeTerrain_tag = AreaTag::Plains;
	m_activeTerrain_soundID = 0;

	// music
	AudioController::getInstance()->stop(m_activeTerrain_soundID);

	PathFindingThread::unlock();

	AudioController::getInstance()->flush();
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
		dt *= 0.45;
	return dt;
}

float Scene::getDeltaTime_skipSlow() { return m_timer.getDt(); }

bool SceneAbstactContent::load_raw(string folder) {
	string path = path_scenes + folder;
	ifstream file;
	file.open(path+"/scene.data", ios::in | ios::binary);
	if (file.is_open()) {
		//leaderboard
		fileRead(file, m_leaderboardName);
		// terrain
		size_t size = fileRead<size_t>(file);
		m_terrainFiles.resize(size);
		for (size_t i = 0; i < size; i++)
			fileRead(file, m_terrainFiles[i]);
		// seas
		size = fileRead<size_t>(file);
		m_seaAreas.resize(size);
		for (size_t i = 0; i < size; i++)
			fileRead<SeaContent>(file, m_seaAreas[i]);
		// particlesystems
		size = fileRead<size_t>(file);
		m_particleSystemContents.resize(size);
		for (size_t i = 0; i < size; i++)
			fileRead<ParticleSystemContent>(file, m_particleSystemContents[i]);
		// entities
		size = fileRead<size_t>(file);
		m_entities.resize(size);
		for (size_t i = 0; i < size; i++) {
			fileRead(file, m_entities[i].model);
			size_t subSize = fileRead<size_t>(file);
			m_entities[i].instances.resize(subSize);
			file.read(
				(char*)m_entities[i].instances.data(), sizeof(GroupInstance::Instance) * subSize);
		}
		//animals
		size = fileRead<size_t>(file);
		m_animals.resize(size);
		for (size_t i = 0; i < size; i++) {
			fileRead<AnimalContent>(file, m_animals[i]);
		}
		// utility
		fileRead<SceneUtilityInfo>(file, m_utility);

		file.close();
		return true;
	}
	return false;
}

bool SceneAbstactContent::save_raw(string folder) { 
	string path = path_scenes + folder;
	create_directory(path);
	ofstream file;
	file.open(path+"/scene.data", ios::out | ios::binary);
	if (file.is_open()) {
		//leaderboard
		fileWrite(file, m_leaderboardName);
		//terrain
		fileWrite<size_t>(file, m_terrainFiles.size());
		for (size_t i = 0; i < m_terrainFiles.size(); i++)
			fileWrite(file, m_terrainFiles[i]);
		//seas
		fileWrite<size_t>(file, m_seaAreas.size());
		for (size_t i = 0; i < m_seaAreas.size(); i++)
			file.write((char*)&m_seaAreas[i], sizeof(SeaContent));
		//particlesystems
		fileWrite<size_t>(file, m_particleSystemContents.size());
		for (size_t i = 0; i < m_particleSystemContents.size(); i++)
			fileWrite<ParticleSystemContent>(file, m_particleSystemContents[i]);
		//entities
		fileWrite<size_t>(file, m_entities.size());
		for (size_t i = 0; i < m_entities.size(); i++) {
			fileWrite(file, m_entities[i].model);
			fileWrite<size_t>(file, m_entities[i].instances.size());
			file.write((char*)m_entities[i].instances.data(),
				sizeof(GroupInstance::Instance) * m_entities[i].instances.size());
		}
		//animals
		fileWrite<size_t>(file, m_animals.size());
		for (size_t i = 0; i < m_animals.size(); i++) {
			fileWrite<AnimalContent>(file, m_animals[i]);
		}
		//utility
		fileWrite<SceneUtilityInfo>(file, m_utility);

		file.close();
		return true;
	}
	return false;
}

SceneAbstactContent::SceneAbstactContent() { }
