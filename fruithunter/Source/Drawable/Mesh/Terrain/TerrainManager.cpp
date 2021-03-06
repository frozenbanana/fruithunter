#include "TerrainManager.h"

TerrainManager TerrainManager::m_this;

TerrainManager* TerrainManager::getInstance() { return &m_this; }

TerrainManager::TerrainManager() {}
void TerrainManager::add(float3 position, float3 scale, string heightmapFilename,
	vector<string> textures, XMINT2 subSize, XMINT2 division, float3 wind) {

	Terrain terrain(heightmapFilename, textures, subSize, division, wind);
	terrain.setPosition(position);
	terrain.setScale(scale);
	m_terrains.push_back(terrain);
}

void TerrainManager::removeAll() { m_terrains.clear(); }


Terrain* TerrainManager::getTerrainFromPosition(float3 position) {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i].pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return &m_terrains[i];
		}
	}
	return nullptr;
}

int TerrainManager::getTerrainIndexFromPosition(float3 position) {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i].pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return (int)i;
		}
	}
	return -1;
}

float TerrainManager::getHeightFromPosition(float3 position) {
	float highest = 0;
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float h = m_terrains[i].getHeightFromPosition(position.x, position.z);
		if (h > highest)
			highest = h;
	}
	return highest;
}

float3 TerrainManager::getNormalFromPosition(float3 position) {
	float highest = 0;
	float3 normal = float3(0, 1, 0);
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float h = m_terrains[i].getHeightFromPosition(position.x, position.z);
		if (h > highest) {
			highest = h;
			normal = m_terrains[i].getNormalFromPosition(position.x, position.z);
		}
	}
	return normal;
}

float TerrainManager::castRay(float3 point, float3 direction) {
	float intersection = -1;
	for (size_t i = 0; i < m_terrains.size(); i++) {
		float l = m_terrains[i].castRay(point, direction);
		if (l != -1 && (intersection == -1 || l < intersection)) {
			intersection = l;
		}
	}
	return intersection;
}

void TerrainManager::clearCulling() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i].clearCulling();
}

void TerrainManager::quadtreeCull(const vector<FrustumPlane>& planes) {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i].quadtreeCull(planes);
}

void TerrainManager::draw() {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		m_terrains[i].draw();
	}
}

void TerrainManager::draw_onlyMesh() {
	for (size_t i = 0; i < m_terrains.size(); i++)
		m_terrains[i].draw_onlyMesh();
}

float3 TerrainManager::getSpawnpoint(int terrainType) {
	return m_terrains[terrainType].getRandomSpawnPoint();
}
