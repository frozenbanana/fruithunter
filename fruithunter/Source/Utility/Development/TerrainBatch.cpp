#include "TerrainBatch.h"

size_t TerrainBatch::length() const { return m_terrains.size(); }

void TerrainBatch::remove(size_t index) {
	if (index >= 0 && index < m_terrains.size()) {
		m_terrains.erase(m_terrains.begin()+index);
	}
}

void TerrainBatch::add(float3 position, float3 scale, string heightmapFilename,
	string textures[4], XMINT2 subSize, XMINT2 division, float3 wind, AreaTag tag) {
	
	m_terrains.push_back(make_shared<Terrain>());//create new
	m_terrains.back()->initilize(heightmapFilename, textures, subSize, division, wind, tag);
	m_terrains.back()->setPosition(position);
	m_terrains.back()->setScale(scale);
}

void TerrainBatch::clear() { m_terrains.clear(); }

Terrain* TerrainBatch::getTerrainFromPosition(float3 position) {
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

shared_ptr<Terrain> TerrainBatch::getTerrainFromIndex(size_t index) {
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

void TerrainBatch::draw_onlyMesh() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i]->draw_onlyMesh();
}

float3 TerrainBatch::getSpawnpoint(size_t terrainIndex) {
	return m_terrains[terrainIndex]->getRandomSpawnPoint();
}
