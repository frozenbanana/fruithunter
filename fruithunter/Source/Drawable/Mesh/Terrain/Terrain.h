#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"
#include "QuadTree.h"
#include "Transformation.h"
#include "TextureRepository.h"
#include "Fragment.h"
#include "SimpleDirectX.h"
#include "HeightmapMesh.h"

class Terrain : public Transformation, public Fragment {
private:
	//-------------------------- GRASS ------------------------------
	// Constants
	const int SETTING_SLOT = 5;
	const int TEX_NOISE_SLOT = 0;
	const int CBUFFER_ANIMATION_SLOT = 6;
	const int CBUFFER_NOISESIZE_SLOT = 9;
	const int STRAW_PER_AREAUNIT = 120;

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
	shared_ptr<TextureSet> m_tex_noise;
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
	const int SAMPLER_SLOT = 0;

	// Heightmap handler
	HeightmapMesh m_heightmapMesh;

	// Shaders
	static ShaderSet m_shader_terrain;
	static ShaderSet m_shader_terrain_onlyMesh;
	static ShaderSet m_shader_terrain_brush;

	// quadtree for culling
	QuadTree<XMINT2> m_quadtree;

	// resource buffer
	const string m_texturePath = "assets/Meshes/Textures/";
	// 0 = flat, 1 = lowFlat, 2 = tilt, 3 = lessTilt
	bool m_textureInitilized = false;
	shared_ptr<TextureSet> m_textures[4];

	// Sampler description
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

	// brush
	static ConstantBuffer<Brush> m_buffer_brush;

	//-------------------------- GENERAL ------------------------------
	// Constants
	const int MATRIX_SLOT = 0;

	struct SubGrid {
	private:
		const bool FLAT_SHADING = true;
		const bool EDGE_SHADING = false;
		const float EDGE_THRESHOLD = 0.3f;
		// terrain
		vector<Vertex> m_vertices;
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

	// buffers
	void createTerrainTextureSampler();

	// grids
	void fillSubMeshes();

	// resource
	bool createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer);

	bool boxInsideFrustum(float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes); // unused!!

	XMINT2 getGridPointSize() const;

public:
	// settings
	void setTextures(string textures[4]);
	void setStrawAndAnimationSettings(AreaTag tag);

	// get settings
	void getTextures(string textures[4]) const;
	XMINT2 getSplits() const;
	XMINT2 getSubSize() const;

	// terrain scanning
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
		string filename, string textures[4], XMINT2 subsize, XMINT2 splits = XMINT2(1, 1));
	void build(string heightmapName, XMINT2 subSize, XMINT2 splits);
	void changeSize(XMINT2 tileSize, XMINT2 gridSize);
	void loadFromFile_binary(fstream& file);
	void storeToFile_binary(fstream& file);

	void editMesh(const Brush& brush, Brush::Type type);
	void editMesh_push();
	void editMesh_pop();
	void editMesh_clear();

	Terrain(const Terrain& other);
	Terrain(string filename = "", string textures[4] = nullptr, XMINT2 subsize = XMINT2(0, 0),
		XMINT2 splits = XMINT2(1, 1));
	~Terrain();

	Terrain& operator=(const Terrain& other);
};