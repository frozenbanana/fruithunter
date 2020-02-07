#pragma once
#include "Renderer.hpp"
#include "ErrorLogger.hpp"
#include "ShaderSet.hpp"
#include "MeshHandler.hpp"

#define MATRIX_BUFFER_SLOT 0

class Terrain {
private:
	struct SubGrid {
	private:
		vector<Vertex> m_vertices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	public:
		static float triangleTest(
			float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

		void initilize(
			XMINT2 tileSize, vector<vector<float>>& map, vector<vector<float3>>& mapNormal);
		void createBuffers();
	public:
		vector<Vertex>* getPtr();
		void initilize();
		void bind();
		unsigned int getVerticeCount() const;

		SubGrid();
		~SubGrid();
		
	
		bool castRay(float3& point, float3& direction);
	};

	const string m_heightmapPath = "TerrainHeightmap/";

	bool m_isInitilized = false;
	static ShaderSet m_shader;
	float3 m_position;
	float3 m_rotation;
	float3 m_scale = float3(1, 0.25, 1) * 4;
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
	vector<vector<SubGrid>> m_subMeshes;

	//grid points !! USED FOR COLLISION
	XMINT2 m_gridPointSize;
	vector<vector<Vertex>> m_gridPoints;

	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	// resource buffer
	const string m_grassPath = "Meshes/Textures/texture_grass.jpg";
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_map_grass;

	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	//buffers
	void createBuffers();

	//model matrix
	float4x4 getModelMatrix();
	void bindModelMatrix();

	// heightmap
	bool loadHeightmap(string filePath);
	float sampleHeightmap(float2 uv);
	float3 calcNormalFromHeightmap(XMINT2 index);
	float3 sampleHeightmapNormal(float2 uv);

	// grids
	void createGrid(XMINT2 size);
	void createGridPointsFromHeightmap();
	void fillSubMeshes(bool flatShaded = false);

	std::wstring s2ws(const std::string& s);
	string LPWSTR_to_STRING(LPWSTR str);

	//resource
	bool createResourceBuffer(string path, ID3D11ShaderResourceView** buffer);

	void tileRayIntersectionTest(XMINT2 gridIndex, float3 point, float3 direction, float& minL, float3& normal);
	float clamp(float val, float min, float max) { 
		return (val<min?min:val>max?max:val);
	}

public:
	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);
	static float triangleTest(float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

	void initilize(string filename, XMINT2 subsize, XMINT2 splits = XMINT2(1, 1));

	void rotateY(float radian);
	void setScale(float3 scale);

	float getHeightFromPosition(float x, float z);
	float3 getNormalFromPosition(float x, float z);
	bool castRay(float3& point, float3& direction);

	void draw();

	Terrain(string filename = "", XMINT2 subsize = XMINT2(0, 0), XMINT2 splits = XMINT2(1, 1));
	~Terrain();
};
