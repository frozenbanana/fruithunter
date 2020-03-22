#pragma once
#include "ShaderSet.h"
#include "GlobalNamespaces.h"
#include "Mesh.h"

struct lightInfo {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

class SkyBox {
private:
	ShaderSet m_shaderSkyBox;
	Mesh m_box;

	AreaTag m_oldLight = AreaTag::Plains;
	AreaTag m_newLight = AreaTag::Plains;

	//interpolation buffer
	float m_interpolation = 1.f;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_interpolationBuffer;

	//textures
	const string m_prePath = "assets/Meshes/Textures/"; 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textures[AreaTag::NR_OF_AREAS];

	//light
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
	lightInfo m_lightInfo[AreaTag::NR_OF_AREAS];
	lightInfo m_interpolatedLightInfo;

	// -- Functions --

	bool createResourceBuffer(string path, ID3D11ShaderResourceView** buffer);
	bool createConstantBuffer();
	wstring s2ws(const std::string& s);
	bool bindTextures();
	bool bindTexture(AreaTag tag);

	bool createLightBuffer();
	void createShaders();

	void updateLightInfo();
	void updateLightInfo(AreaTag tag);

public:
	//manual binding
	void bindLightBuffer();
	//Light information
	bool switchLight(AreaTag tag);
	//draw
	void draw();
	void draw(AreaTag tag);
	//update
	void update(float dt);
	//initilizers
	void load(string fileNames[AreaTag::NR_OF_AREAS], lightInfo lightInfos[AreaTag::NR_OF_AREAS]);
	void loadStandard();
	SkyBox();
	~SkyBox();
};