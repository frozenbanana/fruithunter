#pragma once
#include "Terrain.h"
#include "GrassManager.h"

/*
 * Manages external factors to Terrain
*/
class Environment : public Terrain {
private:
	float3 m_wind;
	AreaTag m_tag = AreaTag::Plains;
	int m_fruitSpawn[NR_OF_FRUITS]{ 0 };

public:
	Environment(string filename = "", string textures[4] = nullptr, XMINT2 subsize = XMINT2(0, 0),
		XMINT2 splits = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f),
		AreaTag tag = AreaTag::Plains);

	void setWind(float3 wind);
	void setTag(AreaTag tag);
	void setFruitSpawns(int fruitSpawns[NR_OF_FRUITS]);

	float3 getWindStatic() const;
	AreaTag getTag() const;
	int getFruitCount(FruitType type) const;

	float3 getRandomSpawnPoint();

	void loadFromBinFile(string path);
	void storeToBinFile(string path);
};

/*
 * Handles the Terrains as if they were one and the same, making the terrain interaction easier.
 */
class TerrainBatch {
private:
	vector<shared_ptr<Environment>> m_terrains;

public:
	size_t length() const;

	void remove(size_t index);
	void add(float3 position, float3 scale, string heightmapFilename, string textures[4], XMINT2 subSize, XMINT2 division = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f), AreaTag tag = AreaTag::Plains);
	void add(shared_ptr<Environment> environment);
	void clear();
	Environment* getTerrainFromPosition(float3 position);
	int getTerrainIndexFromPosition(float3 position) const;
	shared_ptr<Environment> getTerrainFromIndex(size_t index);
	float getHeightFromPosition(float3 position) const;
	float3 getNormalFromPosition(float3 position) const;
	float castRay(float3 point, float3 direction);

	void clearCulling();
	void quadtreeCull(const vector<FrustumPlane>& planes);

	void draw();
	void draw_brush(const Brush& brush);
	void draw_onlyMesh();
	void draw_grass();

	void editMesh(const Brush& brush, Brush::Type type);
	void editMesh_pop();
	void editMesh_push();

	float3 getSpawnpoint(size_t terrainIndex);

};
