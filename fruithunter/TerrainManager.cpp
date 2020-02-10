#include "TerrainManager.h"

void TerrainManager::add(float3 position, string heightmapFilename, XMINT2 subSize, XMINT2 division) {
	Terrain terrain(heightmapFilename,subSize,division);
	terrain.setPosition(position);
	terrain.setScale(float3(1,0.25,1)*10);
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

void TerrainManager::draw() {
	for (size_t i = 0; i < m_terrains.size(); i++) {
		m_terrains[i].draw();
	}
}
