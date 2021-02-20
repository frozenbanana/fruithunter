#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#include "QuadTree.h"
#include "Transformation.h"
#include "Fragment.h"

class SeaEffect : public Transformation, public Fragment {
public:
	enum SeaEffectTypes { water, lava, Count };

private:
	const int CBUFFER_MATRIX_SLOT = 0;
	const int CBUFFER_TIME_SLOT = 2;
	const int CBUFFER_PROPERTIES_SLOT = 3;
	const int CBUFFER_MAPSSIZES_SLOT = 4;

	struct SubWaterGrid {
	private:
		vector<Vertex> m_vertices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

		void createBuffers();

	public:
		void initilize();
		vector<Vertex>* getPtr();
		void bind();
		size_t getVerticeCount() const;

		SubWaterGrid();
		~SubWaterGrid();
	};
	vector<vector<SubWaterGrid>> m_grids;
	XMINT2 m_tileSize, m_gridSize;

	// quadtree
	QuadTree<XMINT2> m_quadtree;
	bool m_useCulling = false;
	vector<XMINT2*> m_culledGrids;

	// shader variables
	static ShaderSet m_shader;
	static ShaderSet m_shader_onlyMesh;

	// constant buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_propertiesBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_timeBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_mapSizesBuffer;

	float4 m_time;
	struct WaterShaderProperties {
		float distortionStrength = 0.01f;
		float whiteDepthDifferenceThreshold = 0.1f;
		float timeSpeed = 0.05f;
		UINT32 waterShadingLevels = 10;
		float depthDifferenceStrength = 0.4f;
		float3 color_edge = float3(0.8f, 0.8f, 1.f);
		float4 color_shallow = float4(12.f / 255.f, 164.f / 255.f, 255.f / 255.f, 1.f);
		float4 color_deep = float4(0.f / 255.f, 71.f / 255.f, 114.f / 255.f, 1.f);
		float2 heightThreshold_edge = float2(0.46f, 0.48f);
		float2 tideHeightScaling = float2(0.3f, 1.f);
		float tideHeightStrength = 2.f;
		float3 filler;
	} m_properties;
	SeaEffectTypes m_type;
	struct WaterShaderMapSizes {
		XMINT2 dudvMapSize;
		XMINT2 heightmapSize;
	} m_mapSizes;

	// maps
	const string m_texturePath = "assets/Meshes/Textures/";
	const string m_waterHeightMapName = "waterHeightmap.png";
	const string m_dudvMapName = "dudvMap.jpg";
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_waterHeightMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_dudvMap;

	//-- Functions --
	void createVertices(XMINT2 tiles, XMINT2 gridSize);
	void createBuffers();

	void bindConstantBuffers();

	bool createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer);
	XMINT2 getResourceSize(ID3D11ShaderResourceView* view);

	bool boxInsideFrustum(
		float3 boxPos, float3 boxSize, float4x4 worldMatrix, const vector<FrustumPlane>& planes);

public:
	void setType(SeaEffectTypes type);

	SeaEffectTypes getType() const;
	XMINT2 getTileSize() const;
	XMINT2 getGridSize() const;

	void update(float dt);

	void clearCulling();
	void quadtreeCull(vector<FrustumPlane> planes);
	void boundingBoxCull(CubeBoundingBox bb);

	void draw();
	void draw_onlyMesh();
	void draw_frustumCulling(const vector<FrustumPlane>& planes);
	void draw_quadtreeFrustumCulling(vector<FrustumPlane> planes);
	void draw_quadtreeBBCulling(CubeBoundingBox bb);

	void build(XMINT2 tiles, XMINT2 gridSize);
	void initilize(SeaEffectTypes type, XMINT2 tiles, XMINT2 gridSize = XMINT2(1, 1),
		float3 position = float3(0, 0, 0), float3 scale = float3(1, 1, 1),
		float3 rotation = float3(0, 0, 0));

	SeaEffect();
	~SeaEffect();
};