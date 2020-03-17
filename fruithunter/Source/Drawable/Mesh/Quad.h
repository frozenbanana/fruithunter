#pragma once
#include "GlobalNamespaces.h"
#include "ShaderSet.h"

class Quad {
public:
	Quad();
	Quad(string texturePath);
	void setTexturePath(string path);
	void setPixelShaderPath(string path);
	~Quad();
	void init();
	void draw();
	void draw(ID3D11ShaderResourceView* rsv);

private:
	void createMesh();
	void createShaders();
	wstring m_texturePath;
	wstring m_pixelShaderPath;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	ShaderSet m_shader;
};
