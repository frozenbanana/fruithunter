#pragma once
#include "ShaderSet.h"
#include "GlobalNamespaces.h"
#include "Mesh.h"

class SkyBox {
private:
	ShaderSet m_shaderSkyBox;
	Mesh m_box;
	float m_dt;
	string m_fileNames[4] = { "ForestSkybox.jpg", "DesertSkybox.jpg", "PlainsSkybox.jpg",
		"VolcanoSkybox.jpg" };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textures[4];
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

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
};