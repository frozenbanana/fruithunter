#pragma once
#include "Terrain.h"

class TerrainManager {
public:
	static TerrainManager* getInstance();
	void add(float3 position, float3 scale, string heightmapFilename, vector<string> textures, XMINT2 subSize,
		XMINT2 division = XMINT2(1, 1));
	Terrain* getTerrainFromPosition(float3 position);
	int getTerrainIndexFromPosition(float3 position);
	float getHeightFromPosition(float3 position);
	float3 getNormalFromPosition(float3 position);
	float castRay(float3 point, float3 direction);

	void draw();
	void drawShadow();

	float3 getSpawnpoint(int terrainType);

private:
	TerrainManager();
	vector<Terrain> m_terrains;
	static TerrainManager m_this;
};
