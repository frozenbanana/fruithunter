#include "Quad.hpp"

struct Vertex{
	float x, y;
	float r, g, b;
};

Quad::Quad(Renderer& renderer)
{
	createMesh(renderer);
	createShaders(renderer);
	createRenderStates(renderer);
}

Quad::~Quad() {
	m_vertexBuffer->Release();
	m_vertexShader->Release();
	m_pixelShader->Release();
	m_inputLayout->Release();
    m_depthState->Release();
	m_blendState->Release();
}

void Quad::draw(Renderer& renderer)
{
	auto deviceContext = renderer.getDeviceContext();

	// Set render states
	deviceContext->RSSetState(m_rasterizerState);
	deviceContext->OMSetBlendState(m_blendState, NULL, 0xffffffff);
	deviceContext->OMSetDepthStencilState(m_depthState, 1);

	// Set shaders to renderer
	deviceContext->IASetInputLayout(m_inputLayout);
	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	// Bind our vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Draw
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->Draw(6, 0);
}

void Quad::createMesh(Renderer& renderer)
{
	// TODO: Make index-based rendering
	Vertex vertices[] = {
		// Triangle 1
		{ -1,  -1,  1, 1, 1 },
		{  1.,  -1,  0, 1, 1 },
		{  1.,   1,  1, 1, 0 },
		// Triangle 2
		{  -1,  -1,  0.5, .2, 1 },
		{  1,  1,  0, 1, .3 },
		{  -1,  1,  0.2, .2, 0.4 },
	};

	// Create our vertex buffer
	auto vertexBufferDesc = CD3D11_BUFFER_DESC(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = vertices;

	renderer.getDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

}

void Quad::createShaders(Renderer& renderer)
{
	auto device = renderer.getDevice();
	ifstream vsFile("VertexShader.cso", ios::binary);
	ifstream psFile("PixelShader.cso", ios::binary);

	// Black magic. But we get the data from file.
	vector<char> vsData = { istreambuf_iterator<char>(vsFile), istreambuf_iterator<char>() };
	vector<char> psData = { istreambuf_iterator<char>(psFile), istreambuf_iterator<char>() };

	device->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &m_vertexShader);
	device->CreatePixelShader(psData.data(), psData.size(), nullptr, &m_pixelShader);

	// Create inut layouts
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
	};

	device->CreateInputLayout(layout, 2, vsData.data(), vsData.size(), &m_inputLayout);
}

void Quad::createRenderStates(Renderer& renderer) {
	// Rasterizer state
	auto rasterizerDesc = CD3D11_RASTERIZER_DESC(
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE,
		false,
		0, 0, 0, 0,
		false, false, false);
	renderer.getDevice()->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

	// Blend state
	auto blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	renderer.getDevice()->CreateBlendState(&blendDesc, &m_blendState);

	// Depth state
	auto depthDesc = CD3D11_DEPTH_STENCIL_DESC(
		FALSE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, 
		FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK, 
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, 
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);
	renderer.getDevice()->CreateDepthStencilState(&depthDesc, &m_depthState);
}
