#include "TerrainManager.h"

void TerrainManager::add(float3 position, string heightmapFilename, vector<string> textures,
	XMINT2 subSize, XMINT2 division) {
	Terrain terrain(heightmapFilename, textures, subSize, division);
	terrain.setPosition(position);
	terrain.setScale(float3(1, 0.25, 1) * 100);
	m_terrains.push_back(terrain);
}


Terrain* TerrainManager::getTerrainFromPosition(float3 position) {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		if (m_terrains[i].pointInsideTerrainBoundingBox(position)) {
			return &m_terrains[i];
		}
	}
	return nullptr;
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
	float3 normal = float3(0,1,0);
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
