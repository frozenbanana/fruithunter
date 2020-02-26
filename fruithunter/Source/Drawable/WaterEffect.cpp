#include "WaterEffect.h"
#include "WICTextureLoader.h"
#include "VariableSyncer.h"

ShaderSet WaterEffect::m_shader;

void WaterEffect::createVertices(XMINT2 tiles, XMINT2 gridSize) {
	// create water base mesh
	XMINT2 order[6] = { // tri1
		XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
		// tri2
		XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
	};
	m_grids.resize(gridSize.x);
	for (size_t gx = 0; gx < gridSize.x; gx++) {
		m_grids[gx].resize(gridSize.y);
		for (size_t gy = 0; gy < gridSize.y; gy++) {
			SubWaterGrid* sub = &m_grids[gx][gy];
			sub->getPtr()->reserve(tiles.x * tiles.y * 6);
			float2 uvLength(1.f / (gridSize.x), 1.f / (gridSize.y));
			float2 uvBase(gx * uvLength.x, gy * uvLength.y); 
			for (size_t xx = 0; xx < tiles.x+1; xx++) {
				for (size_t yy = 0; yy < tiles.y+1; yy++) {
					// create triangles
					for (int i = 0; i < 6; i++) {
						XMINT2 index((int32_t)xx + order[i].x, (int32_t)yy + order[i].y);
						float2 uv((float)index.x / tiles.x, (float)index.y / tiles.y);
						float2 globalUV = uvBase + float2(uv.x * uvLength.x, uv.y * uvLength.y);
						sub->getPtr()->push_back(
							Vertex(float3(globalUV.x, 0, globalUV.y), globalUV, float3(0, 1, 0)));
					}
				}
			}
			sub->initilize();
		}
	}

}

void WaterEffect::createBuffers() {
	// matrix buffer
	if (m_worldMatrixBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(WorldMatrixBuffer);

		HRESULT res = Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_worldMatrixBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating matrix buffer in WaterEffect class!\n");
	}
	// time Buffer
	if (m_timeBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(float4);

		HRESULT res = Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_timeBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating time buffer in WaterEffect class!\n");
	}
	// properties Buffer
	if (m_propertiesBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(WaterShaderProperties);

		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_propertiesBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating properties buffer in WaterEffect class!\n");
	}
	// maps sizes Buffer
	if (m_mapSizesBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(WaterShaderMapSizes);

		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_mapSizesBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating map sizes buffer in WaterEffect class!\n");
	}
	//maps
	createResourceBuffer(m_waterHeightMapName, m_waterHeightMap.GetAddressOf());
	createResourceBuffer(m_dudvMapName, m_dudvMap.GetAddressOf());
	m_mapSizes.heightmapSize = getResourceSize(m_waterHeightMap.Get());
	m_mapSizes.dudvMapSize = getResourceSize(m_dudvMap.Get());
}

void WaterEffect::updateMatrix() {
	if (m_matrixPropertiesChanged) {
		m_matrixPropertiesChanged = false;
		// create matrix
		float4x4 translate = float4x4::CreateTranslation(m_position);
		float4x4 scale = float4x4::CreateScale(m_scale);
		float4x4 rotateX = float4x4::CreateRotationX(m_rotation.x);
		float4x4 rotateY = float4x4::CreateRotationY(m_rotation.y);
		float4x4 rotateZ = float4x4::CreateRotationZ(m_rotation.z);
		m_worldMatrix.mWorld = scale * (rotateZ * rotateX * rotateY) * translate;
		m_worldMatrix.mInvWorld = m_worldMatrix.mWorld.Invert();
		// update sub resource
		WorldMatrixBuffer data = m_worldMatrix;
		data.mInvWorld = data.mInvWorld.Transpose();
		data.mWorld = data.mWorld.Transpose();
		Renderer::getDeviceContext()->UpdateSubresource(
			m_worldMatrixBuffer.Get(), 0, 0, &data, 0, 0);
	}
}

void WaterEffect::bindConstantBuffers() {
	auto deviceContent = Renderer::getDeviceContext();
	//matrix
	bindWorldMatrix();

	//time buffer
	deviceContent->UpdateSubresource(m_timeBuffer.Get(), 0, 0, &m_time, 0, 0);
	deviceContent->VSSetConstantBuffers(
		CONSTANTBUFFER_TIME_SLOT, 1, m_timeBuffer.GetAddressOf());
	deviceContent->PSSetConstantBuffers(
		CONSTANTBUFFER_TIME_SLOT, 1, m_timeBuffer.GetAddressOf());

	//properties buffer
	deviceContent->UpdateSubresource(
		m_propertiesBuffer.Get(), 0, 0, &m_properties, 0, 0);
	deviceContent->PSSetConstantBuffers(
		CONSTANTBUFFER_PROPERTIES_SLOT, 1, m_propertiesBuffer.GetAddressOf());
	deviceContent->VSSetConstantBuffers(
		CONSTANTBUFFER_PROPERTIES_SLOT, 1, m_propertiesBuffer.GetAddressOf());

	//map sizes buffer
	deviceContent->UpdateSubresource(m_mapSizesBuffer.Get(), 0, 0, &m_mapSizes, 0, 0);
	deviceContent->VSSetConstantBuffers(
		CONSTANTBUFFER_MAPSSIZES_SLOT, 1, m_mapSizesBuffer.GetAddressOf());
}

void WaterEffect::bindWorldMatrix() { 
	updateMatrix();
	Renderer::getDeviceContext()->VSSetConstantBuffers(
		WORLDMATRIX_BUFFER_SLOT, 1, m_worldMatrixBuffer.GetAddressOf());
}

std::wstring WaterEffect::s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

bool WaterEffect::createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	wstring wstr = s2ws(m_texturePath + filename);
	LPCWCHAR str = wstr.c_str();
	HRESULT hrA = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, buffer);
	if (FAILED(hrA)) {
		ErrorLogger::messageBox(hrA, "Failed creating texturebuffer from texture\n" + filename);
		return false;
	}
	return true;
}

XMINT2 WaterEffect::getResourceSize(ID3D11ShaderResourceView* view) {
	ID3D11Resource* res = nullptr;
	view->GetResource(&res);
	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = res->QueryInterface(&tex);

	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	return XMINT2(desc.Width,desc.Height);
}

float4x4 WaterEffect::getModelMatrix() { 
	return m_worldMatrix.mWorld;
}

void WaterEffect::setPosition(float3 position) {
	m_position = position;
	m_matrixPropertiesChanged = true;
}

void WaterEffect::setScale(float3 scale) {
	m_scale = scale;
	m_matrixPropertiesChanged = true;
}

void WaterEffect::setRotation(float3 rotation) { 
	m_rotation = rotation;
	m_matrixPropertiesChanged = true;
}

void WaterEffect::update(float dt) { m_time.x += dt; }

void WaterEffect::draw() {
	
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	// bind shaders
	m_shader.bindShadersAndLayout();

	// bind world matrix
	bindWorldMatrix();

	//bind depth buffer
	//Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects are drawn
	Renderer::getInstance()->bindDepthSRV(7);

	//bind screen size
	Renderer::getInstance()->bindConstantBuffer_ScreenSize(5);

	//bind constant buffers
	bindConstantBuffers();

	//bind maps
	Renderer::getDeviceContext()->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
	Renderer::getDeviceContext()->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
	Renderer::getDeviceContext()->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());

	for (size_t xx = 0; xx < m_grids.size(); xx++) {
		for (size_t yy = 0; yy < m_grids[xx].size(); yy++) {
			//bind vertex buffer
			m_grids[xx][yy].bind();
			//draw
			Renderer::getDeviceContext()->Draw((UINT)m_grids[xx][yy].getVerticeCount(), 0);
		}
	}
}

void WaterEffect::initilize(SeaEffectTypes type,XMINT2 tiles, XMINT2 gridSize, float3 position,
	float3 scale, float3 rotation) {

	WaterShaderProperties properties;
	switch (type) {
	case WaterEffect::effect_water:
		properties.distortionStrength = 0.01f;
		properties.whiteDepthDifferenceThreshold = 0.1f;
		properties.timeSpeed = 0.05f;
		properties.waterShadingLevels = 10;
		properties.depthDifferenceStrength = 0.4f;
		properties.color_edge = float3(0.8f, 0.8f, 1.f);
		properties.color_shallow = float4(12.f / 255.f, 164.f / 255.f, 255.f / 255.f, 1.f);
		properties.color_deep = float4(0.f / 255.f, 71.f / 255.f, 114.f / 255.f, 1.f);
		properties.heightThreshold_edge = float2(0.46f, 0.48f);
		properties.tideHeightScaling = float2(0.3f, 1.f);
		properties.tideHeightStrength = 2.f;
		break;
	case WaterEffect::effect_lava:
		properties.distortionStrength = 0.03f;
		properties.whiteDepthDifferenceThreshold = 0.1f;
		properties.timeSpeed = 0.02f;
		properties.waterShadingLevels = 10;
		properties.depthDifferenceStrength = 0.3f;
		properties.color_edge = float3(0.5f, 0.0f, 0.0f);
		properties.color_shallow = float4(0.6f, 0.0f, 0.0f, 1.0f);
		properties.color_deep = float4(0.9f, 0.3f, 0.0f, 1.f);
		properties.heightThreshold_edge = float2(0.50f, 0.52f);
		properties.tideHeightScaling = float2(1.0f, 1.f);
		properties.tideHeightStrength = 1.f;
		break;
	default:
		break;
	}
	m_properties = properties;

	setPosition(position);
	setScale(scale);
	setRotation(rotation);

	createVertices(tiles,gridSize);
	createBuffers();
}

WaterEffect::WaterEffect() {
	//variable syncer
	VariableSyncer::getInstance()->create("WaterEffect.txt", nullptr);
	VariableSyncer::getInstance()->bind("WaterEffect.txt", "Distortion Strength:f&White Effect Threshold:f&Time Speed:f&Shading Levels:i&Depth Difference Strength:f&Color Edge:v3&Color Shallow:v4&Color Deep:v4&Height Threshold Edge:v2&Tide Height Scaling:v2&Tide Height Strength:f", &m_properties);

	//static shader stuff
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
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_shader.createShaders(L"VertexShader_WaterEffect.hlsl", nullptr, L"PixelShader_WaterEffect.hlsl", inputLayout_onlyMesh, 3);
	}
}

WaterEffect::~WaterEffect() {}

void WaterEffect::SubWaterGrid::createBuffers() {
	// vertex buffer
	m_vertexBuffer.Reset();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = (UINT)m_vertices.size() * sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = m_vertices.data();
	HRESULT res =
		Renderer::getDevice()->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logError(
			res, "Failed creating vertex buffer in WaterEffect::SubGrid class!\n");
}

void WaterEffect::SubWaterGrid::initilize() { createBuffers(); }

vector<Vertex>* WaterEffect::SubWaterGrid::getPtr() { return &m_vertices; }

void WaterEffect::SubWaterGrid::bind() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

size_t WaterEffect::SubWaterGrid::getVerticeCount() const {
	return m_vertices.size();
}

WaterEffect::SubWaterGrid::SubWaterGrid() {}

WaterEffect::SubWaterGrid::~SubWaterGrid() {}
