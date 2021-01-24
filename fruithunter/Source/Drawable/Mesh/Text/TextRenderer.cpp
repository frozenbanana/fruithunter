#include "TextRenderer.h"
#include "Renderer.h"
#include "ErrorLogger.h"

ShaderSet TextRenderer::m_shader;
Microsoft::WRL::ComPtr<ID3D11Buffer> TextRenderer::m_vertexBuffer;
size_t TextRenderer::m_vertexCount;
Microsoft::WRL::ComPtr<ID3D11Buffer> TextRenderer::m_worldMatrixBuffer;

TextRenderer::TextRenderer() {
	setFont("myfile2.spritefont");
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());

	m_fontPos.y = (float)SCREEN_HEIGHT / 2.f;
	m_fontPos.x = (float)SCREEN_WIDTH / 2.f;

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

TextRenderer::~TextRenderer() { m_spriteBatch.reset(); }

void TextRenderer::setViewSize(XMINT2 size) { createViewBuffers(size); }

void TextRenderer::setFont(string font) {
	string fontPath = "assets/fonts/" + font;
	wstring w_fontPath = wstring(fontPath.begin(), fontPath.end());
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(Renderer::getDevice(), w_fontPath.c_str());

	if (!m_spriteFont.get()) {
		ErrorLogger::log("TextRenderer failed to load font: "+fontPath);
		return;
	}
}


void TextRenderer::setColor(Color color) { 
	m_color = color;
}

void TextRenderer::setScale(float scale) { m_scale = scale; }

void TextRenderer::setRotation(float rotation) { m_rotation = rotation; }

void TextRenderer::setAlignment(HorizontalAlignment ha, VerticalAlignment va) {
	m_alignment = float2(ha, va);
}

float2 TextRenderer::getSize(string text) {
	wstring wText = std::wstring(text.begin(), text.end());
	return float2(m_spriteFont->MeasureString(wText.c_str()));
}

float3 TextRenderer::normalToRotation(float3 normal) {
	float3 dir = normal;
	dir.Normalize();

	float2 mapY(dir.z, dir.x);
	float2 mapX(float2(dir.x, dir.z).Length(), dir.y);
	mapY.Normalize();
	mapX.Normalize();
	float rotY = (mapY.y >= 0) ? (acos(mapY.x)) : (-acos(mapY.x));
	float rotX = -((mapX.y >= 0) ? (acos(mapX.x)) : (-acos(mapX.x)));
	return float3(rotX, rotY, 0);
}

void TextRenderer::createViewBuffers(XMINT2 viewSize) {
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

void TextRenderer::createAndSetVertexBuffer(vector<Vertex> vertices) {
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

void TextRenderer::createMatrixBuffer() {
	// matrix buffer
	if (m_worldMatrixBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(WorldMatrixData);
		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_worldMatrixBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError("(TextRenderer) Failed creating matrix buffer!", res);
	}
}

void TextRenderer::createBuffers() {
	createMatrixBuffer();
	if (m_vertexBuffer.Get() == nullptr) {
		MeshHandler loader;
		vector<Vertex> vertices;
		loader.load("Quad", vertices);
		createAndSetVertexBuffer(vertices);
	}
}

void TextRenderer::updateWorldMatrixBuffer(float3 position, float3 scale, float3 rotation) {
	WorldMatrixData data;
	float4x4 mat = float4x4::CreateScale(scale) *
				   (float4x4::CreateRotationZ(rotation.z) * float4x4::CreateRotationX(rotation.x) *
					   float4x4::CreateRotationY(rotation.y)) *
				   float4x4::CreateTranslation(position);
	data.mWorld = mat.Transpose();
	data.mInvTraWorld = mat.Invert(); // double transposes deletes eachother
	Renderer::getDeviceContext()->UpdateSubresource(m_worldMatrixBuffer.Get(), 0, 0, &data, 0, 0);
}

void TextRenderer::bindWorldMatrixBuffer() {
	Renderer::getDeviceContext()->VSSetConstantBuffers(0, 1, m_worldMatrixBuffer.GetAddressOf());
}

void TextRenderer::bindVertexBuffer() {

	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void TextRenderer::bindTextView(int slot) {
	Renderer::getDeviceContext()->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
}

void TextRenderer::setDepthStateToNull() {
	/*
		spritebatch använder sin egen eller förändrar på något vis dpthstencilstate, vi har ingen
	   sånn så sb "slår av" våran depthbuffer, sätt state till nullptr för att slå på vår db igen
		kalla på denna i alla textrenderarens drawfunctioner.
	*/
	Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}


// By default SpriteBatch uses premultiplied alpha blending, no depth buffer,
// counter clockwise culling, and linear filtering with clamp texture addressing.
// You can change this by passing custom state objects to SpriteBatch::Begin
// See: https://github.com/microsoft/DirectXTK/wiki/SpriteBatch
void TextRenderer::draw(std::string text, float2 pos) {
	m_spriteBatch->Begin(DirectX::SpriteSortMode_BackToFront);
	std::wstring wText = std::wstring(text.begin(), text.end());

	float2 origin =
		0.5f * ((float2)m_spriteFont->MeasureString(wText.c_str()) * (m_alignment + float2(1, 1)));

	//Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;

	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), pos, m_color, m_rotation, origin, m_scale);

	m_spriteBatch->End();
	setDepthStateToNull();
}

// By default SpriteBatch uses premultiplied alpha blending, no depth buffer,
// counter clockwise culling, and linear filtering with clamp texture addressing.
// You can change this by passing custom state objects to SpriteBatch::Begin
// See: https://github.com/microsoft/DirectXTK/wiki/SpriteBatch
void TextRenderer::drawTextInWorld(string text, float3 position, float3 lookAt, float2 size) {
	std::wstring wText = std::wstring(text.begin(), text.end());
	Vector2 textSize = Vector2(m_spriteFont->MeasureString(wText.c_str()));
	if (textSize.x == 0 || textSize.y == 0)
		return;
	float aspectRatio = textSize.y / textSize.x;
	if (!m_initializedViews || (textSize.x > m_size.x+1 || textSize.y > m_size.y+1))
		setViewSize(XMINT2((UINT)textSize.x, (UINT)textSize.y));
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

	m_spriteBatch->Begin(DirectX::SpriteSortMode_BackToFront, nullptr, nullptr,
		Renderer::getInstance()->getDepthDSS());

	float2 center = float2(m_size.x / 2.f, m_size.y / 2.f);

	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), center, m_color, 0.f, textSize / 2.f, m_scale);

	m_spriteBatch->End();
	setDepthStateToNull();

	// load settings (pop)
	deviceContext->OMSetRenderTargets(1, &holdRTV, holdDSV);
	deviceContext->RSSetViewports(1, &holdVp);
	// draw model in world
	m_shader.bindShadersAndLayout();

	size.y *= aspectRatio;
	updateWorldMatrixBuffer(
		position, float3(size.x, size.y, 1.f), normalToRotation(lookAt - position));
	bindWorldMatrixBuffer();

	bindTextView(0);

	bindVertexBuffer();

	Renderer::getInstance()->enableAlphaBlending();
	Renderer::draw(m_vertexCount, 0);
	Renderer::getInstance()->disableAlphaBlending();

	// Release
	if(holdRTV)holdRTV->Release();
	if(holdDSV)holdDSV->Release();
}
