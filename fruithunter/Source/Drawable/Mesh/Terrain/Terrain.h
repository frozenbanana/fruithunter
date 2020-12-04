#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#include "QuadTree.h"
#include "Transformation.h"
#include "TextureRepository.h"
#include "Fragment.h"
#include "SimpleDirectX.h"

#define MATRIX_BUFFER_SLOT 0
#define SAMPLERSTATE_SLOT 0

class Terrain : public Transformation, public Fragment {
public:
	struct Brush {
		float2 position;
		float radius = 1;
		float falloff = 1;
		float strength = 1;
		float3 bufferFiller;
		enum Type {
			Raise,
			Lower,
			Flatten
		};
	};

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

		void operator=(const SubGrid& other);

		SubGrid();
		~SubGrid();
	};

	const string m_heightmapPath = "assets/TerrainHeightmap/";

	bool m_isInitilized = false;
	static ShaderSet m_shader;
	static ShaderSet m_shader_onlyMesh;
	static ShaderSet m_shader_brush;

	// heightmap
	D3D11_TEXTURE2D_DESC m_heightmapDescription;
	D3D11_MAPPED_SUBRESOURCE m_heightmapMappedData;
	const int SMOOTH_STEPS = 1;

	// grid
	XMINT2 m_tileSize = XMINT2(0, 0);
	XMINT2 m_gridSize = XMINT2(0, 0);
	vector<vector<SubGrid>> m_subMeshes;
	const bool FLAT_SHADING = true;
	const bool EDGE_SHADING = false;
	const float EDGE_THRESHOLD = 0.3f;

	// grid points !! USED FOR COLLISION
	XMINT2 m_gridPointSize;
	vector<vector<Vertex>> m_gridPoints;

	// quadtree for culling
	QuadTree<XMINT2> m_quadtree;

	// resource buffer
	const string m_texturePath = "assets/Meshes/Textures/";
	// 0 = flat, 1 = lowFlat, 2 = tilt, 3 = lessTilt
	bool m_textureInitilized = false;
	shared_ptr<TextureSet> m_textures[4];

	// World Matrix
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	// Sampler description
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	// Culling
	vector<XMINT2> m_culledGrids;
	bool m_useCulling = false;

	// brush
	static ConstantBuffer<Brush> m_buffer_brush;

	//	--Functions--

	// buffers
	void createBuffers();

	// heightmap
	bool loadHeightmap(string filePath);
	float sampleHeightmap(float2 uv);

	// grids
	void createGridPointsFromHeightmap();
	void createGridPointBase();
	void setGridPointNormals();
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

	// get settings
	void getTextures(string textures[4]) const;
	XMINT2 getSplits() const;
	XMINT2 getSubSize() const;

	// intersection tests
	static float obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale);
	static float triangleTest(
		float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2);

	// terrain scanning
	bool pointInsideTerrainBoundingBox(float3 point);
	float getHeightFromPosition(float x, float z);
	float getLocalHeightFromUV(float2 uv);
	float3 getNormalFromPosition(float x, float z);
	float castRay(float3 point, float3 direction);

	// culling
	void clearCulling();
	void quadtreeCull(vector<FrustumPlane> planes);
	void boundingBoxCull(CubeBoundingBox bb);

	// drawing
	void draw();
	void draw_onlyMesh();
	void draw_brush(const Brush& brush);

	void initilize(
		string filename, string textures[4], XMINT2 subsize, XMINT2 splits = XMINT2(1, 1));
	void build(string heightmapName, XMINT2 subSize, XMINT2 splits);
	void changeSize(XMINT2 tileSize, XMINT2 gridSize);
	void loadFromFile_binary(fstream& file);
	void storeToFile_binary(fstream& file);

	void editMesh(const Terrain::Brush& brush, Terrain::Brush::Type type);

	Terrain(const Terrain& other);
	Terrain(string filename = "", string textures[4] = nullptr, XMINT2 subsize = XMINT2(0, 0),
		XMINT2 splits = XMINT2(1, 1));
	~Terrain();

	Terrain& operator=(const Terrain& other);
};