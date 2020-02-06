#pragma once
#include "Renderer.hpp"
#include "ErrorLogger.hpp"
#include "ShaderSet.h"
#include "MeshHandler.h"

#define MATRIX_BUFFER_SLOT 0

class Terrain {
private:
	struct SubGrid {
	private:
		vector<Vertex> m_vertices;
		XMINT2 m_heightmapSize;
		vector<vector<float>> m_heightmap;
		vector<vector<float3>> m_heightmapNormal;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	public:
		static float triangleTest(
			float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

		void initilize(XMINT2 tileSize, vector<vector<float>>& map, vector<vector<float3>>& mapNormal);
		void createBuffers();
		unsigned int getVerticeCount() const;
		void bind();
		float getHeightFromPosition(float x, float z);
		float3 getNormalFromPosition(float x, float z);
		bool castRay(float3& point, float3& direction);
		void createMeshFromHeightmap();

		SubGrid();
		~SubGrid();
	};

	const string m_heightmapPath = "TerrainHeightmap/";

	bool m_isInitilized = false;
	static ShaderSet m_shader;
	float3 m_position;
	float3 m_rotation;
	float3 m_scale = float3(1, 0.25, 1)*4;
	bool m_modelMatrixChanged = true;
	struct ModelBuffer {
		float4x4 mWorld, mWorldInvTra;
	};

	// heightmap
	XMINT2 m_heightmapSize;
	vector<vector<float>> m_heightmap;
	vector<vector<float3>> m_heightmapNormals;

	// grid
	XMINT2 m_tileSize;
	XMINT2 m_gridSize;
	vector<vector<SubGrid>> m_grids;

	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	//buffer
	const string m_grassPath = "Meshes/Textures/texture_grass.jpg";
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_map_grass;

	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	void createBuffers();

	float4x4 getSubModelMatrix(XMINT2 gridIndex);
	float4x4 getModelMatrix();
	void bindModelMatrix(XMINT2 gridIndex);

	bool loadHeightmap(string filePath);
	float sampleHeightmap(float2 uv);
	float3 calcNormalFromHeightmap(XMINT2 index);
	float3 sampleHeightmapNormal(float2 uv);

	void createGrid(XMINT2 size);

	void loadGrids();
	void createSubHeightmap(XMINT2 tileSize, XMINT2 gridIndex);

	std::wstring s2ws(const std::string& s);
	string LPWSTR_to_STRING(LPWSTR str);

	bool createResourceBuffer(string path, ID3D11ShaderResourceView** buffer);

public:
	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);

	void initilize(string filename, XMINT2 subsize, XMINT2 splits = XMINT2(1, 1));

	void rotateY(float radian);
	void setScale(float3 scale);

	float getHeightFromPosition(float x, float z);
	float3 getNormalFromPosition(float x, float z);
	bool castRay(float3& point, float3& direction);

	void draw();

	Terrain(string filename = "", XMINT2 subsize = XMINT2(0, 0), XMINT2 splits = XMINT2(1,1));
	~Terrain();
};
