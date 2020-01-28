#pragma once
#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <WICTextureLoader.h>
#include <wrl/client.h>
#include "ShaderSet.h"
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
	void createRenderStates();

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	ShaderSet m_shader;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
};
