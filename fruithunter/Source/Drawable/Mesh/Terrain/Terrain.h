#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#include "QuadTree.h"
#include "Transformation.h"
#include "TextureRepository.h"
#include "Fragment.h"

#define MATRIX_BUFFER_SLOT 0
#define SAMPLERSTATE_SLOT 0

class Terrain : public Transformation, public Fragment {
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

	const string m_heightmapPath = "assets/TerrainResources/heightmaps/";

	bool m_isInitilized = false;
	static ShaderSet m_shader;
	static ShaderSet m_shader_onlyMesh;

	// heightmap
	D3D11_TEXTURE2D_DESC m_heightmapDescription;
	D3D11_MAPPED_SUBRESOURCE m_heightmapMappedData;
	const int SMOOTH_STEPS = 1;
	string m_heightmapFilename = "";

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

	// quadtree for culling
	QuadTree<XMINT2> m_quadtree;

	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	// resource buffer
	const string m_texturePath = "assets/TerrainResources/";
	// 0 = flat, 1 = lowFlat, 2 = tilt, 3 = lessTilt
	bool m_textureInitilized = false;
	shared_ptr<TextureSet> m_textures[4];

	shared_ptr<TextureSet> m_texture_color[2];
	shared_ptr<TextureSet> m_texture_occlusion[2];
	shared_ptr<TextureSet> m_texture_roughness[2];

	// World Matrix
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	// Sampler description
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	// Spawn point
	vector<float3> m_spawnPoint; // local spawnpoints for fruits

	// Wind
	float3 m_wind;

	// tag
	AreaTag m_tag;

	// Culling
	vector<XMINT2> m_culledGrids;
	bool m_useCulling = false;

	//	--Functions--

	// buffers
	void createBuffers();

	// heightmap
	bool loadHeightmap(string filePath);
	float sampleHeightmap(float2 uv);

	// grids
	void createGrid(XMINT2 size);
	void createGridPointsFromHeightmap();
	void fillSubMeshes();

	// resource
	bool createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer);

	void tileRayIntersectionTest(XMINT2 gridIndex, float3 point, float3 direction, float& minL);
	float clamp(float val, float min, float max) {
		return (val < min ? min : val > max ? max : val);
	}

	bool boxInsideFrustum(float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes);

public:
	// settings
	void setTextures(string textures[4]);
	void setWind(float3 wind);
	void setTag(AreaTag tag);

	// get settings
	void getTextures(string textures[4]) const;
	XMINT2 getSplits() const;
	XMINT2 getSubSize() const;
	string getLoadedHeightmapFilename() const;

	// convenable functions
	float3 getRandomSpawnPoint();
	float3 getWindFromPosition(float3 position);

	// intersection tests
	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);
	static float triangleTest(
		float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

	// terrain scanning
	bool pointInsideTerrainBoundingBox(float3 point);
	float getHeightFromPosition(float x, float z);
	float3 getNormalFromPosition(float x, float z);
	float castRay(float3 point, float3 direction);

	float3 getWindStatic() const;
	AreaTag getTag() const;

	// culling
	void clearCulling();
	void quadtreeCull(vector<FrustumPlane> planes);
	void boundingBoxCull(CubeBoundingBox bb);

	// drawing
	void draw();
	void draw_onlyMesh();

	void initilize(string filename, string textures[4], XMINT2 subsize,
		XMINT2 splits = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f),
		AreaTag tag = AreaTag::Plains);
	void build(string heightmapName, XMINT2 subSize, XMINT2 splits);

	Terrain(string filename = "", string textures[4] = nullptr, XMINT2 subsize = XMINT2(0, 0),
		XMINT2 splits = XMINT2(1, 1), float3 wind = float3(0.f, 0.f, 0.f),
		AreaTag tag = AreaTag::Plains);
	~Terrain();
};