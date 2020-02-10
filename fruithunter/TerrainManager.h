#pragma once
#include "Terrain.h"
class TerrainManager {
private:
	vector<Terrain> m_terrains;

public:

	void add(float3 position, string heightmapFilename, XMINT2 subSize, XMINT2 division = XMINT2(1,1));
	Terrain* getTerrainFromPosition(float3 position);

	void draw();
};
