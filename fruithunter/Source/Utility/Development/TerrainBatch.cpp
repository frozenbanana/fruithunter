#include "TerrainBatch.h"

size_t TerrainBatch::length() const { return m_terrains.size(); }

void TerrainBatch::remove(size_t index) {
	if (index >= 0 && index < m_terrains.size()) {
		m_terrains.erase(m_terrains.begin()+index);
	}
}

void TerrainBatch::add(float3 position, float3 scale, string heightmapFilename,
	string textures[4], XMINT2 subSize, XMINT2 division, float3 wind, AreaTag tag) {
	
	m_terrains.push_back(make_shared<Environment>());//create new
	m_terrains.back()->initilize(heightmapFilename, textures, subSize, division);
	m_terrains.back()->setPosition(position);
	m_terrains.back()->setScale(scale);
	m_terrains.back()->setWind(wind);
	m_terrains.back()->setTag(tag);
}

void TerrainBatch::add(shared_ptr<Environment> environment) { m_terrains.push_back(environment); }

void TerrainBatch::clear() { m_terrains.clear(); }

Environment* TerrainBatch::getTerrainFromPosition(float3 position) {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i]->pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return m_terrains[i].get();
		}
	}
	return nullptr;
}

int TerrainBatch::getTerrainIndexFromPosition(float3 position) const { 
for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i]->pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return (int)i;
		}
	}
	return -1;
}

shared_ptr<Environment> TerrainBatch::getTerrainFromIndex(size_t index) {
	return m_terrains[index];
}

float TerrainBatch::getHeightFromPosition(float3 position) const {
	float highest = 0;
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float h = m_terrains[i]->getHeightFromPosition(position.x, position.z);
		if (h > highest)
			highest = h;
	}
	return highest;
}

float3 TerrainBatch::getNormalFromPosition(float3 position) const { 
float highest = 0;
	float3 normal = float3(0, 1, 0);
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float h = m_terrains[i]->getHeightFromPosition(position.x, position.z);
		if (h > highest) {
			highest = h;
			normal = m_terrains[i]->getNormalFromPosition(position.x, position.z);
		}
	}
	return normal;
}

float TerrainBatch::castRay(float3 point, float3 direction) { 
	float intersection = -1;
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float l = m_terrains[i]->castRay(point, direction);
		if (l != -1 && (intersection == -1 || l < intersection)) {
			intersection = l;
		}
	}
	return intersection;
}

void TerrainBatch::clearCulling() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->clearCulling();
}

void TerrainBatch::quadtreeCull(const vector<FrustumPlane>& planes) {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->quadtreeCull(planes);
}

void TerrainBatch::draw() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->draw();
}

void TerrainBatch::draw_brush(const Brush& brush) {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->draw_brush(brush);
}

void TerrainBatch::draw_onlyMesh() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->draw_onlyMesh();
}

void TerrainBatch::draw_grass() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->draw_grass();
}

void TerrainBatch::editMesh(const Brush& brush, Brush::Type type) {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->editMesh(brush,type);
}

void TerrainBatch::editMesh_pop() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->editMesh_pop();
}

void TerrainBatch::editMesh_push() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->editMesh_push();
}

float3 TerrainBatch::getSpawnpoint(size_t terrainIndex) {
	return m_terrains[terrainIndex]->getRandomSpawnPoint();
}

Environment::Environment(
	string filename, string textures[4], XMINT2 subsize, XMINT2 splits, float3 wind, AreaTag tag) : Terrain(filename, textures, subsize, splits) {
	setWind(wind);
	setTag(tag);
}

void Environment::setWind(float3 wind) { m_wind = wind; }

void Environment::setTag(AreaTag tag) {
	m_tag = tag;
	setStrawAndAnimationSettings(tag);
}

void Environment::setFruitSpawns(int fruitSpawns[NR_OF_FRUITS]) {
	memcpy(m_fruitSpawn, fruitSpawns, sizeof(int) * NR_OF_FRUITS);
}

float3 Environment::getWindStatic() const { return m_wind; }

AreaTag Environment::getTag() const { return m_tag; }

int Environment::getFruitCount(FruitType type) const { return m_fruitSpawn[type]; }

float3 Environment::getRandomSpawnPoint() {
	float3 point;
	float3 normal;
	size_t iterator = 0;
	size_t tries = 100;
	do {
		point = float3::Transform(float3(RandomFloat(0, 1), 0, RandomFloat(0, 1)), getMatrix());
		point.y = getHeightFromPosition(point.x, point.z);
		normal = getNormalFromPosition(point.x, point.z);
		iterator++;
	} while (iterator < tries && (normal.Dot(float3(0, 1, 0)) < 0.75f || point.y < 0.5f));
	if (iterator == tries)
		ErrorLogger::logError("(Environment) Failed finding a spawn point for fruit!",HRESULT());
	return point;
}

void Environment::loadFromBinFile(string path) {
	fstream file;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		// area
		file.read((char*)&m_tag, sizeof(AreaTag));
		// wind
		file.read((char*)&m_wind, sizeof(float3));
		// fruit spawns
		file.read((char*)m_fruitSpawn, sizeof(int) * NR_OF_FRUITS);
		// terrain
		loadFromFile_binary(file);
		setStrawAndAnimationSettings(m_tag);

		file.close();
	}
	else
		ErrorLogger::logError(
			"(Environment) Failed loading environment from file! path: " + path, HRESULT());
}

void Environment::storeToBinFile(string path) {
	fstream file;
	file.open(path, ios::out | ios::binary);
	if (file.is_open()) {
		// area
		file.write((char*)&m_tag, sizeof(AreaTag));
		// wind
		file.write((char*)&m_wind, sizeof(float3));
		// fruit spawns
		file.write((char*)m_fruitSpawn, sizeof(int)*NR_OF_FRUITS);
		// terrain
		storeToFile_binary(file);

		file.close();
	}
	else
		ErrorLogger::logError(
			"(Environment) Failed saving environment to file! path: " + path, HRESULT());
}
