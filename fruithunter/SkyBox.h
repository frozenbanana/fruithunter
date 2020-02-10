#pragma once
#include "ShaderSet.h"
#include "GlobalNamespaces.h"

class SkyBox {
private:
	SkyBox();
	~SkyBox();
	void initialise();
	void draw();

public:
	void createBox();
	void createShaders();

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateSkyBox;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferSkyBox;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferSkyBox;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_textureSkyBox;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceViewSkyBox;
	ShaderSet m_shaderSkyBox;
};