#pragma once
#include "Terrain.h"
class TerrainManager {
private:
	vector<Terrain> m_terrains;

public:

	void add(float3 position, string heightmapFilename, vector<string> textures, XMINT2 subSize,
		XMINT2 division = XMINT2(1, 1));
	Terrain* getTerrainFromPosition(float3 position);
	float getHeightFromPosition(float3 position);
	float3 getNormalFromPosition(float3 position);
	float castRay(float3 point, float3 direction);

	void draw();
};
