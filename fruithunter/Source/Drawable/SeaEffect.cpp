#include "SeaEffect.h"
#include "WICTextureLoader.h"
#include "VariableSyncer.h"
#include "Renderer.h"
#include "ErrorLogger.h"

ShaderSet SeaEffect::m_shader;
ShaderSet SeaEffect::m_shader_onlyMesh;

void SeaEffect::createVertices(XMINT2 tiles, XMINT2 gridSize) {
	size_t layers = (size_t)round(log2(max(gridSize.x, gridSize.y)));
	m_quadtree.initilize(float3(0, 0, 0), float3(1.f, 1.f, 1.f), layers);
	m_quadtree.reserve((size_t)gridSize.x * (size_t)gridSize.y);
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
			for (size_t xx = 0; xx < tiles.x + 1; xx++) {
				for (size_t yy = 0; yy < tiles.y + 1; yy++) {
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

			m_quadtree.add(float3(uvBase.x, 0.f, uvBase.y), float3(uvLength.x, 1.f, uvLength.y),
				XMINT2((int)gx, (int)gy));
		}
	}
}

void SeaEffect::createBuffers() {
	// time Buffer
	if (m_timeBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(float4);

		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_timeBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError("Failed creating time buffer in WaterEffect class!\n", res);
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
			ErrorLogger::logError("Failed creating properties buffer in WaterEffect class!\n", res);
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
			ErrorLogger::logError("Failed creating map sizes buffer in WaterEffect class!\n", res);
	}
	// maps
	createResourceBuffer(m_waterHeightMapName, m_waterHeightMap.GetAddressOf());
	createResourceBuffer(m_dudvMapName, m_dudvMap.GetAddressOf());
	m_mapSizes.heightmapSize = getResourceSize(m_waterHeightMap.Get());
	m_mapSizes.dudvMapSize = getResourceSize(m_dudvMap.Get());
}

void SeaEffect::bindConstantBuffers() {
	auto deviceContent = Renderer::getDeviceContext();
	// matrix
	VSBindMatrix(WORLDMATRIX_BUFFER_SLOT);

	// time buffer
	deviceContent->UpdateSubresource(m_timeBuffer.Get(), 0, 0, &m_time, 0, 0);
	deviceContent->VSSetConstantBuffers(CONSTANTBUFFER_TIME_SLOT, 1, m_timeBuffer.GetAddressOf());
	deviceContent->PSSetConstantBuffers(CONSTANTBUFFER_TIME_SLOT, 1, m_timeBuffer.GetAddressOf());

	// properties buffer
	deviceContent->UpdateSubresource(m_propertiesBuffer.Get(), 0, 0, &m_properties, 0, 0);
	deviceContent->PSSetConstantBuffers(
		CONSTANTBUFFER_PROPERTIES_SLOT, 1, m_propertiesBuffer.GetAddressOf());
	deviceContent->VSSetConstantBuffers(
		CONSTANTBUFFER_PROPERTIES_SLOT, 1, m_propertiesBuffer.GetAddressOf());

	// map sizes buffer
	deviceContent->UpdateSubresource(m_mapSizesBuffer.Get(), 0, 0, &m_mapSizes, 0, 0);
	deviceContent->VSSetConstantBuffers(
		CONSTANTBUFFER_MAPSSIZES_SLOT, 1, m_mapSizesBuffer.GetAddressOf());
}

bool SeaEffect::createResourceBuffer(string filename, ID3D11ShaderResourceView** buffer) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	wstring wstr = s2ws(m_texturePath + filename);
	LPCWCHAR str = wstr.c_str();
	HRESULT hrA = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, buffer);
	if (FAILED(hrA)) {
		ErrorLogger::messageBox(
			hrA, "(SeaEffect) Failed creating texturebuffer from texture\n" + filename);
		return false;
	}
	return true;
}

XMINT2 SeaEffect::getResourceSize(ID3D11ShaderResourceView* view) {
	ID3D11Resource* res = nullptr;
	view->GetResource(&res);
	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = res->QueryInterface(&tex);

	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	return XMINT2(desc.Width, desc.Height);
}

bool SeaEffect::boxInsideFrustum(
	float3 boxPos, float3 boxSize, float4x4 worldMatrix, const vector<FrustumPlane>& planes) {

	float4x4 mWorld = worldMatrix;
	// normalized box points
	float3 boxPoints[8] = { float3(0, 0, 0), float3(1, 0, 0), float3(1, 0, 1), float3(0, 0, 1),
		float3(0, 1, 0), float3(1, 1, 0), float3(1, 1, 1), float3(0, 1, 1) };
	// transform points to world space
	for (size_t i = 0; i < 8; i++) {
		boxPoints[i] =
			float3::Transform(boxPos + float3(boxPoints[i].x * boxSize.x,
										   boxPoints[i].y * boxSize.y, boxPoints[i].z * boxSize.z),
				mWorld);
	}
	// for each plane
	for (size_t plane_i = 0; plane_i < planes.size(); plane_i++) {
		// find diagonal points
		float3 boxDiagonalPoint1, boxDiagonalPoint2;
		float largestDot = -1;
		for (size_t j = 0; j < 4; j++) {
			float3 p1 = boxPoints[j];
			float3 p2 = boxPoints[4 + (j + 2) % 4];
			float3 pn = p1 - p2;
			pn.Normalize();
			float dot = abs(pn.Dot(planes[plane_i].m_normal));
			if (dot > largestDot) {
				largestDot = dot;
				boxDiagonalPoint1 = p1;
				boxDiagonalPoint2 = p2;
			}
		}
		// compare points
		float min = (boxDiagonalPoint1 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		float max = (boxDiagonalPoint2 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		if (min > max) {
			// switch
			float temp = max;
			max = min;
			min = temp;
		}
		if (min > 0) {
			// outside
			return false;
		}
	}
	return true;
}

void SeaEffect::update(float dt) { m_time.x += dt; }

void SeaEffect::clearCulling() {
	m_useCulling = false;
	m_culledGrids.clear();
}

void SeaEffect::quadtreeCull(vector<FrustumPlane> planes) {
	m_useCulling = true;
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();
	for (size_t i = 0; i < planes.size(); i++) {
		planes[i].m_position = float3::Transform(planes[i].m_position, invWorldMatrix);
		planes[i].m_normal = float3::TransformNormal(planes[i].m_normal, invWorldInvTraMatrix);
		planes[i].m_normal.Normalize();
	}
	// cull grids
	m_culledGrids = m_quadtree.cullElements(planes);
}

void SeaEffect::boundingBoxCull(CubeBoundingBox bb) {
	m_useCulling = true;
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();

	bb.m_position = float3::Transform(bb.m_position, invWorldMatrix);
	bb.m_size = bb.m_size / getScale();
	// cull grids
	m_culledGrids = m_quadtree.cullElements(bb);
}

void SeaEffect::draw() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	// bind shaders
	m_shader.bindShadersAndLayout();
	// bind depth buffer
	// Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects are
	// drawn
	Renderer::getInstance()->bindDepthSRVCopy(7);
	// bind screen size
	Renderer::getInstance()->bindConstantBuffer_ScreenSize(7);
	// bind constant buffers
	bindConstantBuffers();
	// bind maps
	Renderer::getDeviceContext()->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
	Renderer::getDeviceContext()->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
	Renderer::getDeviceContext()->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
	// draw
	if (m_useCulling) {
		for (size_t i = 0; i < m_culledGrids.size(); i++) {
			XMINT2 index = *m_culledGrids[i];
			// bind vertex buffer
			m_grids[index.x][index.y].bind();
			// draw
			Renderer::draw((UINT)m_grids[index.x][index.y].getVerticeCount(), 0);
		}
	}
	else {
		for (size_t xx = 0; xx < m_grids.size(); xx++) {
			for (size_t yy = 0; yy < m_grids[xx].size(); yy++) {
				// bind vertex buffer
				m_grids[xx][yy].bind();
				// draw
				Renderer::draw((UINT)m_grids[xx][yy].getVerticeCount(), 0);
			}
		}
	}
}

void SeaEffect::draw_onlyMesh() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	// bind shaders
	m_shader_onlyMesh.bindShadersAndLayout();
	// bind depth buffer
	// Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects are
	// drawn
	Renderer::getInstance()->bindDepthSRVCopy(7);
	// bind screen size
	Renderer::getInstance()->bindConstantBuffer_ScreenSize(5);
	// bind constant buffers
	bindConstantBuffers();
	// bind maps
	Renderer::getDeviceContext()->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
	Renderer::getDeviceContext()->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
	Renderer::getDeviceContext()->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
	// draw
	if (m_useCulling) {
		for (size_t i = 0; i < m_culledGrids.size(); i++) {
			XMINT2 index = *m_culledGrids[i];
			// bind vertex buffer
			m_grids[index.x][index.y].bind();
			// draw
			Renderer::draw((UINT)m_grids[index.x][index.y].getVerticeCount(), 0);
		}
	}
	else {
		for (size_t xx = 0; xx < m_grids.size(); xx++) {
			for (size_t yy = 0; yy < m_grids[xx].size(); yy++) {
				// bind vertex buffer
				m_grids[xx][yy].bind();
				// draw
				Renderer::draw((UINT)m_grids[xx][yy].getVerticeCount(), 0);
			}
		}
	}
}

void SeaEffect::draw_frustumCulling(const vector<FrustumPlane>& planes) {
	float4x4 matrix = getMatrix();
	if (boxInsideFrustum(float3(0, 0, 0), float3(1.f, 1.f, 1.f), matrix, planes)) {
		ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
		// bind shaders
		m_shader.bindShadersAndLayout();
		// bind depth buffer
		// Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects
		// are drawn
		Renderer::getInstance()->bindDepthSRVCopy(7);
		// bind screen size
		Renderer::getInstance()->bindConstantBuffer_ScreenSize(5);
		// bind constant buffers
		bindConstantBuffers();
		// bind maps
		deviceContext->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
		deviceContext->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());

		float3 size(1.f / (float)m_grids.size(), 1.f, 1.f / (float)m_grids[0].size());
		for (size_t xx = 0; xx < m_grids.size(); xx++) {
			for (size_t yy = 0; yy < m_grids[xx].size(); yy++) {
				if (boxInsideFrustum(float3(size.x * (float)xx, size.y, size.z * (float)yy), size,
						matrix, planes)) {
					// bind vertex buffer
					m_grids[xx][yy].bind();
					// draw
					Renderer::draw((UINT)m_grids[xx][yy].getVerticeCount(), 0);
				}
			}
		}
	}
}

void SeaEffect::draw_quadtreeFrustumCulling(vector<FrustumPlane> planes) {
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();
	for (size_t i = 0; i < planes.size(); i++) {
		planes[i].m_position = float3::Transform(planes[i].m_position, invWorldMatrix);
		planes[i].m_normal = float3::TransformNormal(planes[i].m_normal, invWorldInvTraMatrix);
		planes[i].m_normal.Normalize();
	}
	// cull grids
	vector<XMINT2*> elements = m_quadtree.cullElements(planes);
	if (elements.size() > 0) {
		ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
		// bind shaders
		m_shader.bindShadersAndLayout();
		// bind depth buffer
		// Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects
		// are drawn
		Renderer::getInstance()->bindDepthSRVCopy(7);
		// bind screen size
		Renderer::getInstance()->bindConstantBuffer_ScreenSize(5);
		// bind constant buffers
		bindConstantBuffers();
		// bind maps
		deviceContext->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
		deviceContext->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());

		for (size_t i = 0; i < elements.size(); i++) {
			XMINT2 index = *elements[i];
			// bind vertex buffer
			m_grids[index.x][index.y].bind();
			// draw
			Renderer::draw((UINT)m_grids[index.x][index.y].getVerticeCount(), 0);
		}
	}
}

void SeaEffect::draw_quadtreeBBCulling(CubeBoundingBox bb) {
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();
	float3 scale = getScale();
	bb.m_position = float3::Transform(bb.m_position, invWorldMatrix);
	bb.m_size = float3(bb.m_size.x / scale.x, bb.m_size.y / scale.y, bb.m_size.z / scale.z);
	// cull grids
	vector<XMINT2*> elements = m_quadtree.cullElements(bb);
	if (elements.size() > 0) {
		ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
		// bind shaders
		m_shader.bindShadersAndLayout();
		// bind depth buffer
		// Renderer::getInstance()->copyDepthToSRV(); // dont do this here if multiple waterEffects
		// are drawn
		Renderer::getInstance()->bindDepthSRVCopy(7);
		// bind screen size
		Renderer::getInstance()->bindConstantBuffer_ScreenSize(5);
		// bind constant buffers
		bindConstantBuffers();
		// bind maps
		deviceContext->VSSetShaderResources(0, 1, m_waterHeightMap.GetAddressOf());
		deviceContext->VSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, m_dudvMap.GetAddressOf());

		for (size_t i = 0; i < elements.size(); i++) {
			XMINT2 index = *elements[i];
			// bind vertex buffer
			m_grids[index.x][index.y].bind();
			// draw
			Renderer::draw((UINT)m_grids[index.x][index.y].getVerticeCount(), 0);
		}
	}
}

void SeaEffect::initilize(SeaEffectTypes type, XMINT2 tiles, XMINT2 gridSize, float3 position,
	float3 scale, float3 rotation) {

	WaterShaderProperties properties;
	switch (type) {
	case SeaEffect::water:
		properties.distortionStrength = 0.001f;
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
	case SeaEffect::lava:
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

	createVertices(tiles, gridSize);
	createBuffers();
}

SeaEffect::SeaEffect() : Transformation() {
	// static shader stuff
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
		m_shader.createShaders(L"VertexShader_seaEffect.hlsl", nullptr,
			L"PixelShader_seaEffect.hlsl", inputLayout_onlyMesh, 3);
		m_shader_onlyMesh.createShaders(L"VertexShader_seaEffect.hlsl", nullptr,
			L"PixelShader_seaEffect_onlyMesh.hlsl", inputLayout_onlyMesh, 3);
	}
}

SeaEffect::~SeaEffect() {}

void SeaEffect::SubWaterGrid::createBuffers() {
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
			"Failed creating vertex buffer in WaterEffect::SubGrid class!\n", res);
}

void SeaEffect::SubWaterGrid::initilize() { createBuffers(); }

vector<Vertex>* SeaEffect::SubWaterGrid::getPtr() { return &m_vertices; }

void SeaEffect::SubWaterGrid::bind() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

size_t SeaEffect::SubWaterGrid::getVerticeCount() const { return m_vertices.size(); }

SeaEffect::SubWaterGrid::SubWaterGrid() {}

SeaEffect::SubWaterGrid::~SubWaterGrid() {}