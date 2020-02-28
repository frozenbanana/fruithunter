#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#define MATRIX_BUFFER_SLOT 0
#define SAMPLERSTATE_SLOT 0

class Terrain {
private:
	struct SubGrid {
	private:
		vector<Vertex> m_vertices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

		void createBuffers();

	public:
		void initilize();
		vector<Vertex>* getPtr();
		void bind();
		unsigned int getVerticeCount() const;

		SubGrid();
		~SubGrid();
	};

	const string m_heightmapPath = "assets/TerrainHeightmap/";

	bool m_isInitilized = false;
	static ShaderSet m_shader;
	float3 m_position = float3(0, 0, 0);
	float3 m_rotation = float3(0, 0, 0);
	float3 m_scale = float3(1, 0.25, 1) * 100;
	bool m_modelMatrixChanged = true;
	struct ModelBuffer {
		float4x4 mWorld, mWorldInvTra;
	};

	// heightmap
	D3D11_TEXTURE2D_DESC m_heightmapDescription;
	D3D11_MAPPED_SUBRESOURCE m_heightmapMappedData;
	const int SMOOTH_STEPS = 1;

	// grid
	XMINT2 m_tileSize;
	XMINT2 m_gridSize;
	vector<vector<SubGrid>> m_subMeshes;
	const bool FLAT_SHADING = false;
	const bool EDGE_SHADING = false;
	const float EDGE_THRESHOLD = 0.3f;

	// grid points !! USED FOR COLLISION
	XMINT2 m_gridPointSize;
	vector<vector<Vertex>> m_gridPoints;

	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	// resource buffer
	const string m_texturePath = "assets/Meshes/Textures/";
	// 0 = flat, 1 = lowFlat, 2 = tilt, 3 = lessTilt
	const int m_mapCount = 4;
	string m_mapNames[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_maps[4];
	bool m_mapsInitilized = false;

	// World Matrix
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	// Sampler description
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	// Spawn point
	vector<float2> m_spawnPoint;

	float3 m_wind = float3(0.f, 0.f, 10.f);

	//	--Functions--

	// buffers
	void createBuffers();

	// model matrix
	float4x4 getModelMatrix();
	void bindModelMatrix();

	// heightmap
	bool loadHeightmap(string filePath);
	float sampleHeightmap(float2 uv);

	// grids
	void createGrid(XMINT2 size);
	void createGridPointsFromHeightmap();
	void fillSubMeshes();

	std::wstring s2ws(const std::string& s);
	string LPWSTR_to_STRING(LPWSTR str);

	// resource
	bool createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer);

	void tileRayIntersectionTest(XMINT2 gridIndex, float3 point, float3 direction, float& minL);
	float clamp(float val, float min, float max) {
		return (val < min ? min : val > max ? max : val);
	}

public:
	// Spawn point
	float3 getRandomSpawnPoint();

	// Other stuff

	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);
	static float triangleTest(
		float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

	void setPosition(float3 position);

	void initilize(string filename, vector<string> textures, XMINT2 subsize,
		XMINT2 splits = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f));

	void rotateY(float radian);
	void setScale(float3 scale);

	bool pointInsideTerrainBoundingBox(float3 point);
	float getHeightFromPosition(float x, float z);
	float3 getNormalFromPosition(float x, float z);
	float castRay(float3 point, float3 direction);

	float3 getWind();

	void draw();
	void drawShadow();

	Terrain(string filename = "", vector<string> textures = vector<string>(),
		XMINT2 subsize = XMINT2(0, 0), XMINT2 splits = XMINT2(1, 1),
		float3 wind = float3(0.f, 0.f, 0.f));
	~Terrain();
};
