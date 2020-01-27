#pragma once
#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <WICTextureLoader.h>
#include <wrl/client.h>
using namespace std;

class Quad {
public:
	Quad(Renderer& renderer);
	~Quad();
	void draw(Renderer& renderer);

private:
	void createMesh(Renderer& renderer);
	void createShaders(Renderer& renderer);
	void createRenderStates(Renderer& renderer);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState = nullptr;
};
