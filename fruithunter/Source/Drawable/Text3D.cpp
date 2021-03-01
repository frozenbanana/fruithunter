#include "Text3D.h"
#include "Renderer.h"
#include "ErrorLogger.h"

ShaderSet Text3D::m_shader;
Microsoft::WRL::ComPtr<ID3D11Buffer> Text3D::m_vertexBuffer;
size_t Text3D::m_vertexCount;

void Text3D::createViewBuffers(XMINT2 viewSize) {
	m_initializedViews = true;
	m_size = viewSize;

	m_viewport.Width = (float)viewSize.x;
	m_viewport.Height = (float)viewSize.y;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	auto device = Renderer::getDevice();
	// texture 2d copy
	ID3D11Texture2D* tex = 0;
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = viewSize.x;
	texDesc.Height = viewSize.y;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	HRESULT res = device->CreateTexture2D(&texDesc, nullptr, &tex);
	if (FAILED(res))
		ErrorLogger::logError("(TextRenderer) Failed creating Texture2D buffer!", res);
	// resource view
	m_SRV.Reset();
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = texDesc.Format;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;
	descSRV.Texture2D.MostDetailedMip = 0;
	HRESULT srvHR = device->CreateShaderResourceView(tex, &descSRV, m_SRV.GetAddressOf());
	if (FAILED(srvHR))
		ErrorLogger::logError("(TextRenderer) Failed creating SRV buffer!", srvHR);
	// render target
	m_RTV.Reset();
	D3D11_RENDER_TARGET_VIEW_DESC descRTV;
	descRTV.Format = texDesc.Format;
	descRTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	descRTV.Texture2D.MipSlice = 0;
	HRESULT rtvHR = device->CreateRenderTargetView(tex, &descRTV, m_RTV.GetAddressOf());
	if (FAILED(rtvHR))
		ErrorLogger::logError("(TextRenderer) Failed creating RTV buffer!", rtvHR);
	if (tex != nullptr)
		tex->Release();
}

void Text3D::createVertexBuffer(const vector<Vertex>& vertices) {
	// vertex buffer
	m_vertexBuffer.Reset();
	if (m_vertexBuffer.Get() == nullptr) {
		m_vertexCount = vertices.size();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)vertices.size() * sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = vertices.data();

		HRESULT check =
			Renderer::getDevice()->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
		if (FAILED(check))
			ErrorLogger::logError("(TextRenderer) Failed creating vertex buffer!\n", check);
	}
}

void Text3D::createBuffers() {
	if (m_vertexBuffer.Get() == nullptr) {
		MeshHandler loader;
		vector<Vertex> vertices;
		loader.load("Quad", vertices);
		createVertexBuffer(vertices);
	}
}

void Text3D::bindVertexBuffer() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Text3D::bindTextView(int slot) {
	Renderer::getDeviceContext()->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
}

void Text3D::setTarget(float3 target) {
	Transformation::setRotation(vector2Rotation(Normalize(target - Transformation::getPosition())));
}

void Text3D::setText(string text) { 
	if (text != Text2D::getText()) {
		float2 oldSize = Text2D::getLocalSize();
		Text2D::setText(text);
		float2 newSize = Text2D::getLocalSize();
		if (newSize != oldSize) {
			createViewBuffers(XMINT2(newSize.x, newSize.y));
		}
	}
}

void Text3D::setFont(string font) { Text2D::setFont(font); }

void Text3D::setColor(Color color) { Text2D::setColor(color); }

void Text3D::draw() {
	if (m_initializedViews) {
		auto deviceContext = Renderer::getDeviceContext();
		// save settings (push)
		ID3D11RenderTargetView* holdRTV = nullptr;
		ID3D11DepthStencilView* holdDSV = nullptr;
		D3D11_VIEWPORT holdVp;
		UINT vpCount = 1;
		deviceContext->OMGetRenderTargets(1, &holdRTV, &holdDSV);
		deviceContext->RSGetViewports(&vpCount, &holdVp);

		// draw text to render target
		deviceContext->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);
		FLOAT clearColor[4] = { 0, 0, 0, 0 };
		deviceContext->ClearRenderTargetView(m_RTV.Get(), clearColor);
		deviceContext->RSSetViewports(1, &m_viewport);

		// draw text
		Text2D::draw();

		// load settings (pop)
		deviceContext->OMSetRenderTargets(1, &holdRTV, holdDSV);
		deviceContext->RSSetViewports(1, &holdVp);

		// draw model in world
		m_shader.bindShadersAndLayout();

		Transformation form = *this;
		float2 ls = Text2D::getLocalSize();
		float aspectRatio = ls.y / ls.x;
		form.setScale(form.getScale() * float2(1, aspectRatio));
		form.VSBindMatrix(0);

		bindTextView(0);

		bindVertexBuffer();

		Renderer::getInstance()->enableAlphaBlending();
		Renderer::draw(m_vertexCount, 0);
		Renderer::getInstance()->disableAlphaBlending();

		// Release
		if (holdRTV)
			holdRTV->Release();
		if (holdDSV)
			holdDSV->Release();
	}
}

Text3D::Text3D() {
	createBuffers();

	if (!m_shader.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout_onlyMesh[] = {
			{
				"Position",					 // "semantic" name in shader
				0,							 // "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{ "TexCoordinate", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_shader.createShaders(L"VertexShader_onlyMesh.hlsl", nullptr,
			L"PixelShader_textInWorld.hlsl", inputLayout_onlyMesh, 3);
	}
}
