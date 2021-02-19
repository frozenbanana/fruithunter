#pragma once
#include "GlobalNamespaces.h"
#include "Transformation.h"
#include "ShaderSet.h"
#include "Terrain.h"
#include "SimpleDirectX.h"
#include "TextureRepository.h"

class GrassManager : public Transformation {
private:
	const int SETTING_SLOT = 5;
	const int MATRIX_SLOT = 0;
	const int TEX_NOISE_SLOT = 0;
	const int CBUFFER_ANIMATION_SLOT = 6;
	const int CBUFFER_NOISESIZE_SLOT = 9;

	const int STRAW_PER_AREAUNIT = 120;

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
		void operator=(const GrassPatch& other);
	};
	// container
	QuadTree<GrassPatch> m_patches;
	vector<GrassPatch*> m_culledPatches;
	bool m_useCulling = false;

	bool m_visibility = false;
	struct StrawSetting {
		float baseWidth = 0.112f;
		float2 heightRange = float2(0.359f, 0.5f);
		float noiseInterval = 25;
		float4 color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		float4 color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);
	} m_strawSetting;
	ConstantBuffer<StrawSetting> m_cbuffer_settings;
	struct AnimationSetting {
		float time = 0;
		float speed = 20;
		float noiseAnimInterval = 7.4f;
		float offsetStrength = 0.2f;
	} m_animationSetting;
	ConstantBuffer<AnimationSetting> m_cbuffer_animation;

	shared_ptr<TextureSet> m_tex_noise;
	ConstantBuffer<float4> m_cbuffer_noiseSize;

	static ShaderSet m_shader;

	void bindNoiseTexture(size_t slot);

	void update_strawBuffer();
	void update_animationBuffer(float time);


	void imgui_settings();
	void imgui_animation();

public:
	void setStrawAndAnimationSettings(AreaTag tag);

	void clearCulling();
	void quadtreeCull(vector<FrustumPlane> planes);

	void init(Terrain& terrain, AreaTag tag);

	void draw();

	GrassManager();
};
