#include "TerrainManager.h"
#include "PerformanceTimer.h"
TerrainManager TerrainManager::m_this;
TerrainManager* TerrainManager::getInstance() { return &m_this; }
TerrainManager::TerrainManager() {}
void TerrainManager::add(float3 position, float3 scale, string heightmapFilename,
	vector<string> textures, XMINT2 subSize, XMINT2 division, float3 wind) {

	PerformanceTimer::getInstance()->start("Creation of: " + heightmapFilename);
	Terrain terrain(heightmapFilename, textures, subSize, division, wind);
	terrain.setPosition(position);
	terrain.setScale(scale);
	m_terrains.push_back(terrain);
	PerformanceTimer::getInstance()->stop();
}


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

void TerrainManager::draw() {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		m_terrains[i].draw();
	}
}

vector<float3> TerrainManager::draw_frustumCulling(const vector<FrustumPlane>& planes) {
	string strs[4] = { "volcano","forest","desert","plains" };
	for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i].draw_frustumCulling(planes)) {
			//ErrorLogger::log(strs[i]);
		}
	}
	return vector<float3>();
}

void TerrainManager::draw_quadtreeFrustumCulling(const vector<FrustumPlane>& planes) {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		m_terrains[i].draw_quadtreeFrustumCulling(planes);
	}
}

void TerrainManager::drawShadow() {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		m_terrains[i].draw();
	}
}

float3 TerrainManager::getSpawnpoint(int terrainType) {
	return m_terrains[terrainType].getRandomSpawnPoint();
}
