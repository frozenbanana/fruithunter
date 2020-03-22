#include "Quad.h"
#include "ErrorLogger.h"
#include <WICTextureLoader.h>
#include "Renderer.h"

struct Vertex {
	float x, y;
	float r, g, b;
	float u, v;
};

Quad::Quad() {
	string strTex = "assets/Meshes/Textures/loadingScreen.jpg";
	string strPixSha = "PixelShader.hlsl";
	m_texturePath = wstring(strTex.begin(), strTex.end());
	m_pixelShaderPath = wstring(strPixSha.begin(), strPixSha.end());
}

Quad::Quad(string texturePath) {
	wstring wstr(texturePath.begin(), texturePath.end());
	m_texturePath = wstr;
}

void Quad::setTexturePath(string texturePath) {
	wstring wstr(texturePath.begin(), texturePath.end());
	m_texturePath = wstr;
}

void Quad::setPixelShaderPath(string path) {
	wstring wstr(path.begin(), path.end());
	m_pixelShaderPath = wstr;
}

void Quad::init() {
	createMesh();
	createShaders();
	// createRenderStates();
}

Quad::~Quad() {}

void Quad::draw() {
	auto deviceContext = Renderer::getDeviceContext();

	// Set shaders to renderer
	m_shader.bindShadersAndLayout();

	// Set Sampler for texturing
	deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	deviceContext->PSSetShaderResources(0, 1, m_shaderResourceView.GetAddressOf());

	// Bind our vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Bind our index buffer
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Draw
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->DrawIndexed(6, 0, 0);
}

void Quad::createMesh() {
	auto device = Renderer::getDevice();

	// Vertices
	Vertex vertices[] = { { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f } };

	// Create our vertex buffer
	auto vertexBufferDesc = CD3D11_BUFFER_DESC(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = vertices;

	HRESULT vbFlag =
		device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());

	if (FAILED(vbFlag)) {
		ErrorLogger::messageBox("Failed to initalize vertex buffer.");
		return;
	}

	// Indices
	DWORD indices[] = { 0, 1, 2, 0, 3, 2 };

	// Create our index buffer
	auto indexBufferDesc = CD3D11_BUFFER_DESC(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
	D3D11_SUBRESOURCE_DATA indexData = { 0 };
	indexData.pSysMem = indices;

	HRESULT ibFlag =
		device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());

	if (FAILED(ibFlag)) {
		ErrorLogger::messageBox("Failed to initalize vertex buffer.");
		return;
	}

	// Texture
	HRESULT tfFlag = DirectX::CreateWICTextureFromFile(device, m_texturePath.c_str(),
		m_texture.GetAddressOf(), m_shaderResourceView.GetAddressOf());

	if (FAILED(tfFlag)) {
		ErrorLogger::logWarning("Failed to initalize texture from file. Capturing frame", tfFlag);
		Renderer::getInstance()->captureFrame();
	}

	// Sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT ssFlag = device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());

	if (FAILED(ssFlag)) {
		ErrorLogger::messageBox(ssFlag, "Failed to initalize sampler state.");
		return;
	}
}

void Quad::createShaders() {
	// Create input layouts

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA },
	};

	m_shader.createShaders(L"VertexShader.hlsl", nullptr, m_pixelShaderPath.c_str(), layout, 3);
}