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
	float m_dt;
	string m_fileNames[AreaTags::Count] = { "PlainsSkybox.jpg", "ForestSkybox.jpg",
		"DesertSkybox.jpg", "PlainsSkybox.jpg",
		"VolcanoSkybox.jpg" };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textures[AreaTags::Count];
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
	lightInfo m_lightInfo[AreaTags::Count];
	lightInfo m_currentLightInfo;
	AreaTags m_oldLight = AreaTags::None;
	AreaTags m_newLight = AreaTags::None;

	bool createResourceBuffer(string path, ID3D11ShaderResourceView** buffer);
	bool createConstantBuffer();
	wstring s2ws(const std::string& s);
	void bindTextures(int oldSkybox, int newSkybox);

	float clamp(float val, float min, float max) {
		return (val < min ? min : val > max ? max : val);
	}

public:
	SkyBox();
	~SkyBox();

	void draw();
	void createShaders();
	
	void updateDelta(float dt);

	//Light information
	void updateCurrentLight();
	bool updateNewOldLight(AreaTags tag);
	bool createLightBuffer();
	void bindLightBuffer();
};