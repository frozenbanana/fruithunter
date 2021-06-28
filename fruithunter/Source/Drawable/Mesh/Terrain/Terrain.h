#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#include "QuadTree.h"
#include "Transformation.h"
#include "TextureRepository.h"
#include "Fragment.h"
#include "SimpleDirectX.h"
#include "HeightmapMesh.h"

struct TerrainVertex {
	float3 position;
	float2 uv;
	float3 normal;
	float colorIntensity = 1;
	void setFromVertex(const Vertex& v) {
		position = v.position;
		uv = v.uv;
		normal = v.normal;
	}
	TerrainVertex(const Vertex& v) { setFromVertex(v);}
	TerrainVertex(const Vertex& v, float intensity) {
		setFromVertex(v);
		colorIntensity = intensity;
	}
};

class Terrain : public Transformation, public Fragment {
private:
	//-------------------------- GRASS ------------------------------
	// Constants
	const int SETTING_SLOT = 5;
	const int TEX_NOISE_SLOT = 0;
	const int CBUFFER_ANIMATION_SLOT = 6;
	const int CBUFFER_NOISESIZE_SLOT = 9;
	const int STRAW_PER_AREAUNIT = 90;

	// Grass Settings
	bool m_grass_visibility = false;
	struct StrawSetting {
		float baseWidth;
		float2 heightRange;
		float noiseInterval;
		float4 color_top;
		float4 color_bottom;
	} m_grass_strawSetting;
	static ConstantBuffer<StrawSetting> m_cbuffer_settings;
	struct AnimationSetting {
		float time;
		float speed;
		float noiseAnimInterval;
		float offsetStrength;
	} m_grass_animationSetting;
	static ConstantBuffer<AnimationSetting> m_cbuffer_animation;

	// Noise Resource
	shared_ptr<Texture> m_tex_noise;
	static ConstantBuffer<float4> m_cbuffer_noiseSize;

	// Shader
	static ShaderSet m_shader_grass;

	// -- GRASS FUNCTIONS --
	void bindNoiseTexture(size_t slot);

	void update_strawBuffer();
	void update_animationBuffer(float time);

	void imgui_settings();
	void imgui_animation();

	//-------------------------- TERRAIN ------------------------------
	// Constants
	const int CBUFFER_COLOR = 9;

	// Heightmap handler
	HeightmapMesh m_heightmapMesh;

	// Shaders
	static ShaderSet m_shader_terrain;
	static ShaderSet m_shader_terrain_onlyMesh;
	static ShaderSet m_shader_terrain_brush;

	// quadtree for culling
	QuadTree<XMINT2> m_quadtree;

	// brush
	static ConstantBuffer<Brush> m_buffer_brush;

	struct ColorBuffer {
		float4 color_flat = float4(74.0f / 255.f, 106.0f / 255.f, 31.0f / 255.f, 1.0f);
		float4 color_tilt = float4(104.0f / 255.f, 80.0f / 255.f, 29.0f / 255.f, 1.0f);
		float2 intensityRange = float2(0.9f, 1.1f);
		float2 filler;
	} m_colorBuffer;
	static ConstantBuffer<ColorBuffer> m_cbuffer_color;

	void update_colorBuffer();
	void imgui_color();

	//-------------------------- GENERAL ------------------------------
	// Constants
	const int MATRIX_SLOT = 0;

	struct SubGrid {
	private:
		const bool FLAT_SHADING = true;
		const bool EDGE_SHADING = false;
		const float EDGE_THRESHOLD = 0.3f;
		// terrain
		vector<TerrainVertex> m_vertices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vbuffer_terrain;

		// grass
		struct Straw {
			float3 position;
			float rotationY;
			float height;
		};
		vector<Straw> m_straws;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vbuffer_grass;

		void createBuffer_grass();
		void createBuffer_terrain();

	public:
		void generate_terrain(XMINT2 tileSize, XMINT2 gridIndex, HeightmapMesh& hmMesh);
		void generate_grass(float2 position, float2 size, size_t count, HeightmapMesh& hmMesh, float4x4 worldMatrix);

		void bind_terrain();
		void drawCall_terrain();

		void bind_grass();
		void drawCall_grass();

		void operator=(const SubGrid& other);
	};

	// Grid structure
	XMINT2 m_tileSize = XMINT2(0, 0);
	XMINT2 m_gridSize = XMINT2(0, 0);
	vector<vector<SubGrid>> m_subMeshes;

	// Quadtree culling
	vector<XMINT2> m_culledGrids;
	bool m_useCulling = false;

	//----------------------- FUNCTIONS -------------------------

	// grids
	void fillSubMeshes();

	bool boxInsideFrustum(float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes); // unused!!

	XMINT2 getGridPointSize() const;

public:
	// settings
	void setStrawAndAnimationSettings(AreaTag tag);
	void setColorSettings(AreaTag tag);

	// get settings
	XMINT2 getSplits() const;
	XMINT2 getSubSize() const;

	// terrain scanning
	bool validPosition(float3 pos);
	bool pointInsideTerrainBoundingBox(float3 point);
	float getHeightFromPosition(float x, float z);
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
	void draw_grass();

	void initilize(
		string filename, XMINT2 subsize, XMINT2 splits = XMINT2(1, 1));
	void build(string heightmapName, XMINT2 subSize, XMINT2 splits);
	void changeSize(XMINT2 tileSize, XMINT2 gridSize);
	void loadFromFile_binary(fstream& file);
	void storeToFile_binary(fstream& file);

	void editMesh(const Brush& brush, Brush::Type type);
	void editMesh_push();
	void editMesh_pop();
	void editMesh_clear();

	Terrain(const Terrain& other);
	Terrain(string filename = "", XMINT2 subsize = XMINT2(0, 0),
		XMINT2 splits = XMINT2(1, 1));
	~Terrain();

	Terrain& operator=(const Terrain& other);
};