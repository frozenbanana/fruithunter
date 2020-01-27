#include "Quad.hpp"
#include "ErrorLogger.hpp"

struct Vertex {
	float x, y;
	float r, g, b;
	float u, v;
};

Quad::Quad(Renderer& renderer) {
	createMesh(renderer);
	createShaders(renderer);
	createRenderStates(renderer);
}

Quad::~Quad() {}

void Quad::draw(Renderer& renderer) {
	auto deviceContext = renderer.getDeviceContext();
	// auto shaderResourceView = renderer.getShaderResourceView();
	auto samplerState = renderer.getSamplerState();

	// Set render states
	deviceContext->RSSetState(m_rasterizerState.Get());
	deviceContext->OMSetBlendState(m_blendState.Get(), NULL, 0xffffffff);
	deviceContext->OMSetDepthStencilState(m_depthState.Get(), 1);

	// Set shaders to renderer
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// Set Sampler for texturing
	deviceContext->PSSetSamplers(0, 1, &samplerState);
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

void Quad::createMesh(Renderer& renderer) {
	auto device = renderer.getDevice();

	// Vertices
	Vertex vertices[] = { { -1, -1, 1, 1, 1, 1, 1 }, { 1., -1, 0, 1, 1, 0, 1 }, { 1., 1, 1, 1, 0, 0, 0 },
		{
			-1,
			1,
			1,
			1,
			0,
			1,
			0,
		} };

	// Create our vertex buffer
	auto vertexBufferDesc = CD3D11_BUFFER_DESC(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = vertices;

	HRESULT vbFlag = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());

	if (FAILED(vbFlag)) {
		ErrorLogger::log("Failed to initalize vertex buffer.");
		return;
	}

	// Indices
	DWORD indices[] = { 0, 1, 2, 0, 3, 2 };

	// Create our index buffer
	auto indexBufferDesc = CD3D11_BUFFER_DESC(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
	D3D11_SUBRESOURCE_DATA indexData = { 0 };
	indexData.pSysMem = indices;

	HRESULT ibFlag = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());

	if (FAILED(ibFlag)) {
		ErrorLogger::log("Failed to initalize vertex buffer.");
		return;
	}

	// Texture
	HRESULT tfFlag = DirectX::CreateWICTextureFromFile(
		device, L"assets\\goat.jpg", m_texture.GetAddressOf(), m_shaderResourceView.GetAddressOf());

	if (FAILED(tfFlag)) {
		ErrorLogger::log("Failed to initalize texture from file.");
		return;
	}
}

void Quad::createShaders(Renderer& renderer) {
	auto device = renderer.getDevice();
	ifstream vsFile("VertexShader.cso", ios::binary);
	ifstream psFile("PixelShader.cso", ios::binary);

	// Black magic. But we get the data from file.
	vector<char> vsData = { istreambuf_iterator<char>(vsFile), istreambuf_iterator<char>() };
	vector<char> psData = { istreambuf_iterator<char>(psFile), istreambuf_iterator<char>() };

	device->CreateVertexShader(vsData.data(), vsData.size(), nullptr, m_vertexShader.GetAddressOf());
	device->CreatePixelShader(psData.data(), psData.size(), nullptr, m_pixelShader.GetAddressOf());

	// Create inut layouts
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
	};

	HRESULT ilFlag = device->CreateInputLayout(layout, 3, vsData.data(), vsData.size(), m_inputLayout.GetAddressOf());

	if (FAILED(ilFlag)) {
		ErrorLogger::log("Failed to initalize input layout.");
		return;
	}
}

void Quad::createRenderStates(Renderer& renderer) {
	// Rasterizer state
	auto rasterizerDesc =
		CD3D11_RASTERIZER_DESC(D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0, 0, 0, false, false, false);
	renderer.getDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());

	// Blend state
	auto blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	renderer.getDevice()->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());

	// Depth state
	auto depthDesc = CD3D11_DEPTH_STENCIL_DESC(FALSE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, FALSE,
		D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);
	renderer.getDevice()->CreateDepthStencilState(&depthDesc, m_depthState.GetAddressOf());
}
