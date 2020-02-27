#pragma once
#include "ShaderSet.h"
#include "MeshHandler.h"

#define WORLDMATRIX_BUFFER_SLOT 0
#define CONSTANTBUFFER_TIME_SLOT 2
#define CONSTANTBUFFER_PROPERTIES_SLOT 3
#define CONSTANTBUFFER_MAPSSIZES_SLOT 4

class SeaEffect {
private:
	float3 m_position;
	float3 m_scale;
	float3 m_rotation;
	bool m_matrixPropertiesChanged = true;
	struct WorldMatrixBuffer {
		float4x4 mWorld, mInvWorld;
	} m_worldMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_worldMatrixBuffer;

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

	// shader variables
	static ShaderSet m_shader;

	//constant buffers
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
		float4 color_shallow = float4(12.f / 255.f, 164.f / 255.f, 255.f / 255.f,1.f);
		float4 color_deep = float4(0.f / 255.f, 71.f / 255.f, 114.f / 255.f,1.f);
		float2 heightThreshold_edge = float2(0.46f, 0.48f);
		float2 tideHeightScaling = float2(0.3f,1.f);
		float tideHeightStrength = 2.f;
		float3 filler;
	} m_properties;
	struct WaterShaderMapSizes {
		XMINT2 dudvMapSize;
		XMINT2 heightmapSize;
	} m_mapSizes;

	//maps
	const string m_texturePath = "assets/Meshes/Textures/";
	const string m_waterHeightMapName = "waterHeightmap.png";
	const string m_dudvMapName = "dudvMap.jpg";
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_waterHeightMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_dudvMap;

	//-- Functions --
	void createVertices(XMINT2 tiles, XMINT2 gridSize);
	void createBuffers();

	void updateMatrix();

	void bindConstantBuffers();
	void bindWorldMatrix();

	std::wstring s2ws(const std::string& s);
	bool createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer);
	XMINT2 getResourceSize(ID3D11ShaderResourceView* view);

	bool boxInsideFrustum(float3 boxPos, float3 boxSize, float4x4 worldMatrix, const vector<FrustumPlane>& planes);

public:
	enum SeaEffectTypes { water, lava };

	float4x4 getModelMatrix();

	void setPosition(float3 position);
	void setScale(float3 scale);
	void setRotation(float3 rotation);

	void update(float dt);
	void draw();
	void draw_frustumCulling(const vector<FrustumPlane>& planes);

	void initilize(SeaEffectTypes type, XMINT2 tiles, XMINT2 gridSize = XMINT2(1,1), float3 position = float3(0,0,0), float3 scale = float3(1,1,1), float3 rotation = float3(0,0,0));

	SeaEffect();
	~SeaEffect();
};
