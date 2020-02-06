#pragma once
#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <WICTextureLoader.h>
#include <wrl/client.h>
#include "ShaderSet.hpp"
using namespace std;

class Quad {
public:
	Quad();
	~Quad();
	void init();
	void draw();

private:
	void createMesh();
	void createShaders();

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	ShaderSet m_shader;
};
