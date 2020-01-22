#pragma once
#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <WICTextureLoader.h>

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

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	ID3D11Resource* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
	ID3D11RasterizerState* m_rasterizerState = nullptr;
	ID3D11DepthStencilState* m_depthState = nullptr;
	ID3D11BlendState* m_blendState = nullptr;
};
