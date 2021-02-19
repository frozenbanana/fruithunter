#pragma once
#include "GlobalNamespaces.h"
#include "Transformation.h"
#include "ShaderSet.h"
#include "Terrain.h"
#include "SimpleDirectX.h"

#define STRAW_PER_AREAUNIT 120

class GrassManager : public Transformation {
private:
	// buffer
	struct Straw {
		float3 position;
		float rotationY;
		float height;
	};
	// renderer
	struct GrassPatch {
		vector<Straw> m_straws;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		void createBuffer();
		void generate(
			float2 position, float2 size, size_t count, Terrain& terrain);
		void bind();
		void drawCall();
		bool isEmpty() const;
	};
	// container
	QuadTree<GrassPatch> m_patches;
	vector<GrassPatch*> m_culledPatches;
	bool m_useCulling = false;

	struct StrawSetting {
		float baseWidth = 0.072f;
		float minHeight = 0.139f;
		float maxHeight = 0.422f;
		float filler;
		float4 color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		float4 color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);
	} m_strawSetting;
	ConstantBuffer<StrawSetting> m_cbuffer_settings;

	static ShaderSet m_shader;

	void update_strawBuffer();

	void imgui_settings();

public:

	void clearCulling();
	void quadtreeCull(vector<FrustumPlane> planes);

	void init(Terrain& terrain);

	void draw();

	GrassManager();
};
