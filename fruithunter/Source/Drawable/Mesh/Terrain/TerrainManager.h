#pragma once
#include "Terrain.h"

class TerrainManager {
public:
	static TerrainManager* getInstance();
	void add(float3 position, float3 scale, string heightmapFilename, vector<string> textures,
		XMINT2 subSize, XMINT2 division = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f));
	Terrain* getTerrainFromPosition(float3 position);
	int getTerrainIndexFromPosition(float3 position);
	float getHeightFromPosition(float3 position);
	float3 getNormalFromPosition(float3 position);
	float castRay(float3 point, float3 direction);

	void clearCulling();
	void quadtreeCull(const vector<FrustumPlane>& planes);

	void draw();
	void draw_onlyMesh();

	float3 getSpawnpoint(int terrainType);

private:
	TerrainManager();
	vector<Terrain> m_terrains;
	static TerrainManager m_this;
};
