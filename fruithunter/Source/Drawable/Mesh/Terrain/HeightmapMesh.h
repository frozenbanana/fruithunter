#pragma once
#include "GlobalNamespaces.h"
#include "MeshHandler.h"

struct Brush {
	float3 position;
	float radius = 1;
	float falloff = 1;
	float strength = 1;
	float2 bufferFiller;
	enum Type { Raise, Lower, Flatten };
};

class HeightmapMesh {
protected:
	const int EDITMESH_STACKSIZE = 30;
	const string m_heightmapPath = "assets/TerrainHeightmap/";

	const XMINT2 POINT_ORDER[6] = { 
		// tri1
		XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
		// tri2
		XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
	};

	// heightmap texture data
	string m_previousLoaded = "";
	const int SMOOTH_STEPS = 1;

	// grid points
	XMINT2 m_gridPointSize;
	vector<vector<Vertex>> m_gridPoints;

	// editMesh
	vector<shared_ptr<vector<vector<float>>>> m_editMesh_stack; // used to undo modifications 

	// -- FUNCTIONS --

	// heightmap
	static float sampleHeightmap(
		float2 uv, const D3D11_MAPPED_SUBRESOURCE& data, const D3D11_TEXTURE2D_DESC& description);


	// grid
	void createGridPointBase(XMINT2 gridSize);
	bool setGridHeightFromHeightmap(string filename);
	void smoothGrid(size_t iterations);
	void setGridPointNormals();

	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);
	static float triangleTest(
		float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);
	void tileRayIntersectionTest(XMINT2 gridIndex, float3 point, float3 direction, float& minL);

public:
	XMINT2 getSize() const;

	float getHeightFromUV(float2 uv);
	float3 getNormalFromUV(float2 uv);

	float castRay(float3 pointStart, float3 pointEnd);

	bool containsValidPosition(float2 point, float2 size, float4x4 worldMatrix);

	bool editMesh(const Brush& brush, Brush::Type type, float dt, float4x4 matWorld);
	void editMesh_push();
	bool editMesh_pop();
	void editMesh_clear();

	void loadFromFile_binary(fstream& file);
	void storeToFile_binary(fstream& file);

	void changeSize(XMINT2 gridSize);
	bool init(string filename, XMINT2 gridSize);

	vector<Vertex>& operator[](const size_t& index);
	Vertex& operator[](const XMINT2& index);
	HeightmapMesh& operator=(const HeightmapMesh& other);

	HeightmapMesh();

};
