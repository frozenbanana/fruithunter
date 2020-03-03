#include "EntityRepository.h"
#include "TerrainManager.h"
#include "Renderer.h"
#include "ErrorLogger.h"

void EntityRepository::clear() {
	m_repository.clear();
	m_repositoryLoaded = false;
}

void EntityRepository::fillEntitiesFromRepository() {
	if (m_repositoryLoaded) {
		// get count of instances
		int count = 0;
		for (size_t m = 0; m < m_repository.size(); m++)
			count += (int)m_repository[m].instances.size();
		// clear entities and fill from repository
		m_entities.clear();
		m_entities.resize(count);
		int instanceIndex = 0;
		for (size_t m = 0; m < m_repository.size(); m++) {
			string meshName = m_repository[m].meshName;
			for (size_t i = 0; i < m_repository[m].instances.size(); i++) {
				EntityInstance* instance = &m_repository[m].instances[i];
				// fill entity
				m_entities[instanceIndex] = make_unique<Entity>();
				m_entities[instanceIndex]->load(meshName);
				setEntityByInstance(m_entities[instanceIndex].get(), *instance);
				instanceIndex++;
			}
		}
	}
}

void EntityRepository::loadPlacements(string filename) {
	// reset repository to fill with new stuff
	clear();
	// load repository
	string path = m_entityPlacementFilePath + filename + m_fileEndings;
	fstream file;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		// read mesh count
		int meshCount = 0;
		file.read((char*)&meshCount, sizeof(meshCount));
		m_repository.resize(meshCount);
		for (size_t m = 0; m < meshCount; m++) {
			// read mesh name
			char* str = nullptr;
			int meshNameCount = 0;
			file.read((char*)&meshNameCount, sizeof(meshNameCount));
			str = new char[meshNameCount];
			file.read(str, meshNameCount);
			for (size_t i = 0; i < meshNameCount; i++)
				m_repository[m].meshName += str[i]; // set name
			delete[] str;
			// read mesh instance count
			int instanceCount = 0;
			file.read((char*)&instanceCount, sizeof(instanceCount));
			m_repository[m].instances.resize(instanceCount);
			// read instances
			vector<EntityInstance>* instances = &m_repository[m].instances;
			for (size_t i = 0; i < instanceCount; i++) {
				// write instance
				file.read((char*)&instances->at(i), sizeof(EntityInstance));
			}
		}
		file.close();
		m_repositoryLoaded = true;
		m_repositoryFilenameLoadedFrom = filename;
		ErrorLogger::log("(EntityRepository) Loaded entity placements from file: " + filename);
	}
	else {
		ErrorLogger::logWarning(HRESULT(), "Couldnt open and load from file: " + path);
	}
}

void EntityRepository::createFile(string filename) const {
	if (filename != "") {
		string path = m_entityPlacementFilePath + filename + m_fileEndings;
		fstream file;
		file.open(path, ios::out | ios::binary);
		if (file.is_open()) {
			// write a zero! Next time the repository reads from the file it will not crash
			int count = 0;
			file.write((const char*)&count, sizeof(count));
			file.close();
		}
	}
}

bool EntityRepository::fileExists(string filename) const {
	string path = m_entityPlacementFilePath + filename + m_fileEndings;
	fstream file;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		file.close();
		return true;
	}
	return false;
}

float EntityRepository::random(float min, float max) const {
	return min + ((float)(rand() % 1000) / 999.f) * (max - min);
}

void EntityRepository::randomizeProperties(Entity* entity) const {
	entity->setScale(random(0.8f, 1.3f));
	entity->rotateY(random(0.f, 3.14f * 2.f));
}

void EntityRepository::savePlacements(string filename) const {
	if (m_repository.size() > 0 && filename != "") {
		string path = m_entityPlacementFilePath + filename + m_fileEndings;
		fstream file;
		file.open(path, ios::out | ios::binary);
		if (file.is_open()) {
			// write mesh count
			int meshCount = (int)m_repository.size();
			file.write((const char*)&meshCount, sizeof(meshCount));
			for (size_t m = 0; m < meshCount; m++) {
				// write mesh name
				string meshName = m_repository[m].meshName;
				int meshNameCount = (int)meshName.length();
				file.write((const char*)&meshNameCount, sizeof(meshNameCount));
				file.write(meshName.c_str(), meshNameCount);
				// write mesh instance count
				int instanceCount = (int)m_repository[m].instances.size();
				file.write((const char*)&instanceCount, sizeof(instanceCount));
				// write instances
				const vector<EntityInstance>* instances = &m_repository[m].instances;
				for (size_t i = 0; i < instanceCount; i++) {
					// write instance
					file.write((const char*)&instances->at(i), sizeof(EntityInstance));
				}
			}
			file.close();
			ErrorLogger::log("(EntityRepository) Saved entity placements in file: " + filename);
		}
		else {
			ErrorLogger::logWarning(HRESULT(), "Couldnt open and save to file: " + path);
		}
	}
	else {
		// shall not save a repository that is empty!
	}
}

void EntityRepository::load(string filename) {
	if (filename != "") {
		m_castingSphere.load("Sphere");
		m_castingSphere.setScale(0.1f);
		if (fileExists(filename)) {
			loadPlacements(filename);
			fillEntitiesFromRepository();
		}
		else {
			createFile(filename);
			m_repositoryLoaded = true; // the repository didnt exist. So created an empty file
									   // for it and are therefore loaded by an empty file
			m_repositoryFilenameLoadedFrom = filename;
		}
	}
}

void EntityRepository::save() {
	if (m_repositoryLoaded) {
		if (m_repositoryFilenameLoadedFrom != "") {
			if (m_repositoryChangedSinceLoad) {
				savePlacements(m_repositoryFilenameLoadedFrom);
			}
			else {
				ErrorLogger::log(
					"(EntityRepository) Attempt denied to save entity placements to file: " +
					m_repositoryFilenameLoadedFrom + "\nFile already up to date!");
			}
		}
		else
			ErrorLogger::logWarning(
				HRESULT(), "(EntityRepository) Loaded repository name not set when should!");
	}
}

void EntityRepository::addPlaceableEntity(string meshFilename) {
	m_placeable.resize(m_placeable.size() + 1);
	m_placeable.back() = make_unique<Entity>();
	m_placeable.back()->load(meshFilename);
}

void EntityRepository::addEntity(string meshFilename, EntityInstance instance) {
	if (m_repositoryLoaded) {
		// add to entities
		m_entities.resize(m_entities.size() + 1);
		m_entities.back() = make_unique<Entity>();
		m_entities.back()->load(meshFilename);
		setEntityByInstance(m_entities.back().get(), instance);

		// add to repository
		// find
		string meshName = meshFilename;
		bool found = false;
		for (size_t i = 0; i < m_repository.size(); i++) {
			if (meshName == m_repository[i].meshName) {
				m_repository[i].instances.push_back(instance);
				found = true;
			}
		}
		// if not found then add new EntityInstances
		if (!found) {
			m_repository.push_back(EntityInstances(meshName));
			m_repository.back().instances.push_back(instance);
		}
		m_repositoryChangedSinceLoad = true;
	}
}

void EntityRepository::removeEntity(const Entity* entity) {
	if (m_repositoryLoaded && entity != nullptr) {
		string meshName = entity->getModelName();
		EntityInstance instance = getEntityInstance(entity);
		// find entity in m_entities
		for (size_t i = 0; i < m_entities.size(); i++) {
			EntityInstance currInstance = getEntityInstance(m_entities[i].get());
			if (m_entities[i]->getModelName() == meshName && currInstance == instance) {
				m_entities.erase(m_entities.begin() + i);
				break;
			}
		}
		// find entity in m_repository
		for (size_t m = 0; m < m_repository.size(); m++) {
			if (m_repository[m].meshName == meshName) {
				for (size_t i = 0; i < m_repository[m].instances.size(); i++) {
					if (m_repository[m].instances[i] == instance) {
						m_repository[m].instances.erase(m_repository[m].instances.begin() + i);
						break;
					}
				}
			}
		}
		m_repositoryChangedSinceLoad = true;
	}
}

EntityRepository::EntityInstance EntityRepository::getEntityInstance(const Entity* entity) const {
	return EntityInstance(entity->getPosition(), entity->getScale(), entity->getRotationMatrix());
}

void EntityRepository::setEntityByInstance(Entity* entity, EntityInstance instance) {
	entity->setPosition(instance.position);
	entity->setScale(instance.scale);
	entity->setRotationMatrix(instance.matRotation);
}

void EntityRepository::update(float dt, float3 point, float3 direction) {
	Input* ip = Input::getInstance();
	if (ip->keyPressed(m_stateSwitchKey)) {
		// switch state
		m_state = (ModeState)((m_state + 1) % ModeState::Length);
		if (m_state == ModeState::state_placing)
			ErrorLogger::log("Switched state: placing");
		else if (m_state == ModeState::state_removing)
			ErrorLogger::log("Switched state: removing");
		else if (m_state == ModeState::state_inactive)
			ErrorLogger::log("Switched state: inactive");
		//reset variables
		m_markedIndexToRemove = -1;
	}
	if (m_state == ModeState::state_placing) {
		// keys
		if (ip->keyPressed(m_indexIncreaseKey)) {
			// increment up
			m_activePlaceableIndex = (m_activePlaceableIndex + 1) % (int)m_placeable.size();
		}
		if (ip->keyPressed(m_indexDecreaseKey)) {
			// increment down
			m_activePlaceableIndex--;
			if (m_activePlaceableIndex < 0)
				m_activePlaceableIndex = (int)m_placeable.size() - 1;
		}
		if (ip->keyPressed(m_deleteKey)) {
			// delete newest entity
			if (m_entities.size() > 0)
				removeEntity(m_entities.back().get());
		}
		if (ip->mousePressed(m_placeKey)) {
			// place entity
			string meshName = m_placeable[m_activePlaceableIndex]->getModelName();
			EntityInstance instance(m_placeable[m_activePlaceableIndex]->getPosition(),
				m_placeable[m_activePlaceableIndex]->getScale(),
				m_placeable[m_activePlaceableIndex]->getRotationMatrix());
			addEntity(meshName, instance);
			// randomize properties
			randomizeProperties(m_placeable[m_activePlaceableIndex].get());
		}
		if (ip->keyPressed(m_randomizeKey)) {
			// randomize properties
			randomizeProperties(m_placeable[m_activePlaceableIndex].get());
		}
		if (ip->keyPressed(m_saveKey)) {
			// save entities
			save();
		}
		// placement position
		direction.Normalize();
		direction *= m_placingDistance;
		float l = TerrainManager::getInstance()->castRay(point, direction);
		if (l != -1) {
			float3 intersection = point + direction * l;
			m_placeable[m_activePlaceableIndex]->setPosition(intersection);
		}
	}
	else if (m_state == ModeState::state_removing) {
		if (ip->mousePressed(m_placeKey)) {
			float shortestT = -1;
			for (size_t i = 0; i < m_entities.size(); i++) {
				float t = m_entities[i]->castRay(point, direction);
				if ((t > 0 && t < m_placingDistance) && (shortestT == -1 || t < shortestT)) {
					shortestT = t;
					m_markedIndexToRemove = i;
				}
			}
		}
		else if (ip->keyPressed(m_deleteKey)) {
			if (m_markedIndexToRemove != -1) {
				removeEntity(m_entities[m_markedIndexToRemove].get());
				m_markedIndexToRemove = -1;
			}
		}
	}
}

void EntityRepository::draw() {
	for (size_t i = 0; i < m_entities.size(); i++) {
		if (m_markedIndexToRemove == i)
			m_entities[i]->draw_onlyMesh(float3(1.f, 0.f, 0.f));
		else
			m_entities[i]->draw();
	}
	if (m_state == ModeState::state_placing)
		m_placeable[m_activePlaceableIndex]->draw();
}

void EntityRepository::drawShadow() {
	for (size_t i = 0; i < m_entities.size(); i++) {
		m_entities[i]->drawShadow();
	}
	if (m_state == ModeState::state_placing) {
		m_placeable[m_activePlaceableIndex]->drawShadow();
	}
}

EntityRepository::EntityRepository(string filename) { load(filename); }

EntityRepository::~EntityRepository() {}
