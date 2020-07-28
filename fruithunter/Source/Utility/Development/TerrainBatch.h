#pragma once
#include "Terrain.h"

/*
 * Handles the Terrains as if they were one and the same, making the terrain interaction easier.
*/
class TerrainBatch {
private:
	vector<shared_ptr<Terrain>> m_terrains;

public:
	size_t length() const;

	void remove(size_t index);
	void add(float3 position, float3 scale, string heightmapFilename, string textures[4], XMINT2 subSize, XMINT2 division = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f), AreaTag tag = AreaTag::Plains);
	void clear();
	Terrain* getTerrainFromPosition(float3 position);
	int getTerrainIndexFromPosition(float3 position) const;
	shared_ptr<Terrain> getTerrainFromIndex(size_t index);
	float getHeightFromPosition(float3 position) const;
	float3 getNormalFromPosition(float3 position) const;
	float castRay(float3 point, float3 direction);

	void clearCulling();
	void quadtreeCull(const vector<FrustumPlane>& planes);

	void draw();
	void draw_onlyMesh();

	float3 getSpawnpoint(int terrainIndex);

};
