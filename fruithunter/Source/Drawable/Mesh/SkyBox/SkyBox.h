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
	string m_fileNames[4] = { "ForestSkybox.jpg", "DesertSkybox.jpg", "PlainsSkybox.jpg",
		"VolcanoSkybox.jpg" };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textures[4];
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
	lightInfo m_lightInfo[4];
	lightInfo m_currentLightInfo;
	int m_oldLight = 0.0f;
	int m_newLight = 0.0f;

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
	void draw(int oldSkybox, int newSkybox);
	void createShaders();
	
	void updateDelta(float dt);
	void resetDelta();

	//Light information
	void updateCurrentLight();
	void updateNewOldLight(int);
	bool createLightBuffer();
	void bindLightBuffer();
};