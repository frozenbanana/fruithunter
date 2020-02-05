#include "Terrain.h"

ShaderSet Terrain::m_shader;
Microsoft::WRL::ComPtr<ID3D11Buffer> Terrain::m_matrixBuffer;

void Terrain::createBuffers() {
	auto gDevice = Renderer::getDevice();
	// matrix buffer
	if (m_matrixBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(ModelBuffer);

		HRESULT res = gDevice->CreateBuffer(&desc, nullptr, m_matrixBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating matrix buffer in Terrain class!\n");
	}
}

float4x4 Terrain::getSubModelMatrix(XMINT2 gridIndex) {
	float2 subScale(1.0f / m_gridSize.x, 1.0f / m_gridSize.y);
	// float4x4 transform = float4x4::CreateTranslation(
	//	m_position + float3(gridIndex.x * subScale.x, 0, gridIndex.y * subScale.y));
	// float4x4 rotation = float4x4::CreateRotationY(m_rotation.y);
	// float4x4 scale =
	//	float4x4::CreateScale(float3(m_scale.x * subScale.x, m_scale.y, m_scale.z * subScale.y));

	float4x4 transform = float4x4::CreateTranslation(float3(gridIndex.x, 0, gridIndex.y));
	float4x4 scale = float4x4::CreateScale(float3(subScale.x, 1, subScale.y));

	return transform * scale;
}

float4x4 Terrain::getModelMatrix() {
	float4x4 transform = float4x4::CreateTranslation(m_position);
	float4x4 rotation = float4x4::CreateRotationY(m_rotation.y);
	float4x4 scale = float4x4::CreateScale(m_scale);
	return scale * rotation * transform;
}

void Terrain::bindModelMatrix(XMINT2 gridIndex) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	float4x4 mWorld = getSubModelMatrix(gridIndex) * getModelMatrix();
	ModelBuffer matrix;
	matrix.mWorld = mWorld.Transpose();
	matrix.mWorldInvTra =
		mWorld.Invert(); // double transposes does nothing, so the transposes are removed!
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, 0, &matrix, 0, 0);

	deviceContext->VSSetConstantBuffers(MATRIX_BUFFER_SLOT, 1, m_matrixBuffer.GetAddressOf());
}

void Terrain::loadHeightmap(string filePath) {
	auto gDevice = Renderer::getDevice();
	auto gDeviceContext = Renderer::getDeviceContext();

	wstring wstr = s2ws(filePath);
	const wchar_t* fp = wstr.c_str();
	// get texture
	ID3D11Resource* res = nullptr;
	HRESULT hr = CreateWICTextureFromFileEx(gDevice, fp, 0, D3D11_USAGE_STAGING, 0,
		D3D11_CPU_ACCESS_READ, 0, WIC_LOADER_DEFAULT, &res, nullptr);
	if (FAILED(hr)) {
		ErrorLogger::logError(hr, "ERROR! Failed loading texture: " + filePath);
		return;
	}

	ID3D11Texture2D* tex = nullptr;
	hr = res->QueryInterface(&tex);

	D3D11_TEXTURE2D_DESC texD;
	tex->GetDesc(&texD);

	D3D11_MAPPED_SUBRESOURCE sub;
	hr = gDeviceContext->Map(res, 0, D3D11_MAP_READ, 0, &sub);

	int texWidth = texD.Width;
	int texHeight = texD.Height;

	// create
	m_heightmapSize = XMINT2(texWidth, texHeight);
	m_heightmapNormals.resize(m_heightmapSize.x);
	m_heightmap.resize(m_heightmapSize.x);
	for (int i = 0; i < m_heightmapSize.x; i++) {
		m_heightmap[i].resize(m_heightmapSize.y);
		m_heightmapNormals[i].resize(m_heightmapSize.y);
	}

	for (int yy = 0; yy < texWidth; yy++) {
		for (int xx = 0; xx < texHeight; xx++) {
			int ix = xx, iy = yy;
			unsigned char d = 0;
			float v = 0;
			if (texD.Format == DXGI_FORMAT_R8_UNORM) {
				d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix];
				v = (float)d / (pow(2, 1 * 8) - 1);
			}
			else if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix * 4];
				v = (float)d / (pow(2, 1 * 8) - 1);
			}
			else if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix * 4];
				v = (float)d / (pow(2, 1 * 8) - 1);
			}
			else if (texD.Format == DXGI_FORMAT_R16G16B16A16_UNORM) {
				unsigned short int d =
					((unsigned short int*)
							sub.pData)[iy * (sub.RowPitch / sizeof(short int)) + ix * 4];
				v = (float)d / (pow(2, sizeof(short int) * 8) - 1);
			}
			m_heightmap[xx][yy] = v;
		}
	}
	for (int xx = 0; xx < m_heightmapSize.x; xx++) {
		for (int yy = 0; yy < m_heightmapSize.y; yy++) {
			m_heightmapNormals[xx][yy] = calcNormalFromHeightmap(XMINT2(xx, yy));
		}
	}
}

float Terrain::sampleHeightmap(float2 uv) {
	XMINT2 index =
		XMINT2(round(uv.x * (m_heightmapSize.x - 1)), round(uv.y * (m_heightmapSize.y - 1)));
	return m_heightmap[index.x][index.y];
}

float3 Terrain::calcNormalFromHeightmap(XMINT2 index) {
	bool minX = (index.x <= 0);
	bool maxX = (index.x >= m_heightmapSize.x - 1);
	bool minY = (index.y <= 0);
	bool maxY = (index.y >= m_heightmapSize.y - 1);

	float2 pos(1.0f / m_heightmapSize.x, 1.0f / m_heightmapSize.y);
	float3 p = float3(0, m_heightmap[index.x][index.y], 0);
	float3 pTop = float3(0, minY ? 0 : m_heightmap[index.x][index.y - 1.], -pos.y);
	float3 pRight = float3(pos.x, maxX ? 0 : m_heightmap[index.x + 1.][index.y], 0);
	float3 pLeft = float3(-pos.x, minX ? 0 : m_heightmap[index.x - 1.][index.y], 0);
	float3 pBottom = float3(0, maxY ? 0 : m_heightmap[index.x][index.y + 1.], pos.y);

	float3 normal;
	if (!minX || !minY) {
		float3 n = (pTop - p).Cross(pLeft - p);
		n.Normalize();
		normal += n;
	}
	if (!minX || !maxY) {
		float3 n = (pLeft - p).Cross(pBottom - p);
		n.Normalize();
		normal += n;
	}
	if (!maxX || !maxY) {
		float3 n = (pBottom - p).Cross(pRight - p);
		n.Normalize();
		normal += n;
	}
	if (!maxX || !minY) {
		float3 n = (pRight - p).Cross(pTop - p);
		n.Normalize();
		normal += n;
	}
	normal.Normalize();
	return normal;
}

float3 Terrain::sampleHeightmapNormal(float2 uv) {
	XMINT2 index =
		XMINT2(round(uv.x * (m_heightmapSize.x - 1)), round(uv.y * (m_heightmapSize.y - 1)));
	return m_heightmapNormals[index.x][index.y];
}

void Terrain::loadGrids() {
	for (int xx = 0; xx < m_gridSize.x; xx++) {
		for (int yy = 0; yy < m_gridSize.y; yy++) {
			createSubHeightmap(m_tileSize, XMINT2(xx, yy));
		}
	}
}

void Terrain::createGrid(XMINT2 size) {
	if (size.x > 0 && size.y > 0) {
		m_gridSize = size;
		m_grids.resize(m_gridSize.x);
		for (int x = 0; x < m_gridSize.y; x++) {
			m_grids[x].resize(m_gridSize.y);
		}
	}
	else {
		ErrorLogger::logWarning(
			HRESULT(), "WARNING! Tried to change a terrain to inappropriate size");
	}
}

void Terrain::createSubHeightmap(XMINT2 tileSize, XMINT2 gridIndex) {
	if (!(tileSize.x == 0 || tileSize.y == 0)) {
		vector<vector<float>> heightmap;
		vector<vector<float3>> heightmapNormal;
		heightmap.resize(tileSize.x + 1);
		heightmapNormal.resize(tileSize.x + 1.);
		float2 lengthUV(1.0f / m_gridSize.x, 1.0f / m_gridSize.y);
		float2 baseUV(gridIndex.x * lengthUV.x, gridIndex.y * lengthUV.y);
		for (int xx = 0; xx < tileSize.x + 1; xx++) {
			heightmap[xx].resize(tileSize.y+1);
			heightmapNormal[xx].resize(tileSize.y+1.);
			for (int yy = 0; yy < tileSize.y+1; yy++) {
				float2 uv = float2(
					(float)(xx) / tileSize.x, (float)(yy) / tileSize.y);
				float2 globalUV = baseUV + float2(uv.x*lengthUV.x, uv.y*lengthUV.y);

				heightmap[xx][yy] = sampleHeightmap(globalUV);
				heightmapNormal[xx][yy] = sampleHeightmapNormal(globalUV);
			}
		}
		m_grids[gridIndex.x][gridIndex.y].initilize(tileSize,heightmap, heightmapNormal);
	}
}

std::wstring Terrain::s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

string Terrain::LPWSTR_to_STRING(LPWSTR str) {
	wstring ws(str);
	return string(ws.begin(), ws.end());
}

void Terrain::initilize(string filename, XMINT2 subsize, XMINT2 splits) {
	if (filename == "" || (splits.x == 0 || splits.y == 0) || (subsize.x == 0 || subsize.y == 0)) {
		// do nothing
	}
	else {
		m_isInitilized = true;
		m_tileSize = subsize;
		createGrid(splits);						   // create space for memory
		loadHeightmap(m_heightmapPath + filename); // define heightmap
		loadGrids();
		createBuffers();
	}
}

void Terrain::rotateY(float radian) {
	m_modelMatrixChanged = true;
	m_rotation.y += radian;
}

float Terrain::getHeightFromPosition(float x, float z) {
	float3 position(x, 0, z);
	float4x4 mTerrainInvWorld = getModelMatrix().Invert();
	position = float3::Transform(position, mTerrainInvWorld);

	if (position.x >= 0 && position.x <= 1 && position.y >= 0 && position.y <= 1 &&
		position.z >= 0 && position.z <= 1) {

		 for (int xx = 0; xx < m_gridSize.x; xx++) {
			for (int yy = 0; yy < m_gridSize.y; yy++) {
				float4x4 mInvWorld = getSubModelMatrix(XMINT2(xx, yy)).Invert();
				float3 subPosition = float3::Transform(position, mInvWorld);
				if (subPosition.x >= 0 && subPosition.x <= 1 && subPosition.y >= 0 &&
					subPosition.y < 1 && subPosition.z >= 0 && subPosition.z < 1) {
					
					float h = m_grids[xx][yy].getHeightFromPosition(subPosition.x,subPosition.z);
					return h;
				}
			}
		}
	}
	return 0;
}

float3 Terrain::getNormalFromPosition(float x, float z) {
	float3 position(x, 0, z);
	float4x4 mTerrainInvWorld = getModelMatrix().Invert();
	position = float3::Transform(position, mTerrainInvWorld);

	if (position.x >= 0 && position.x <= 1 && position.y >= 0 && position.y <= 1 &&
		position.z >= 0 && position.z <= 1) {

		for (int xx = 0; xx < m_gridSize.x; xx++) {
			for (int yy = 0; yy < m_gridSize.y; yy++) {
				float4x4 mInvWorld = getSubModelMatrix(XMINT2(xx, yy)).Invert();
				float3 subPosition = float3::Transform(position, mInvWorld);
				if (subPosition.x >= 0 && subPosition.x <= 1 && subPosition.y >= 0 &&
					subPosition.y < 1 && subPosition.z >= 0 && subPosition.z < 1) {

					float3 n = m_grids[xx][yy].getNormalFromPosition(subPosition.x, subPosition.z);
					return n;
				}
			}
		}
	}
	return float3(0,0,0);
}

void Terrain::draw() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	m_shader.bindShadersAndLayout();

	for (int xx = 0; xx < m_gridSize.x; xx++) {
		for (int yy = 0; yy < m_gridSize.y; yy++) {
			bindModelMatrix(XMINT2(xx, yy));
			m_grids[xx][yy].bind();
			deviceContext->Draw(m_grids[xx][yy].getVerticeCount(), 0);
		}
	}
}

Terrain::Terrain(string filename, XMINT2 subsize, XMINT2 splits) {
	initilize(filename, subsize, splits);
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
		m_shader.createShaders(L"VertexShader_model_onlyMesh.hlsl", NULL,
			L"PixelShader_terrain.hlsl", inputLayout_onlyMesh, 3);
	}
}

Terrain::~Terrain() {}

void Terrain::SubGrid::initilize(
	XMINT2 tileSize, vector<vector<float>>& map, vector<vector<float3>>& mapNormal) {

	m_heightmapSize = XMINT2(tileSize.x + 1, tileSize.y + 1);
	m_heightmap = map;
	m_heightmapNormal = mapNormal;
	createMeshFromHeightmap();
}

void Terrain::SubGrid::createBuffers() {
	// vertex buffer
	m_vertexBuffer.Reset();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = m_vertices.size() * sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = m_vertices.data();
	HRESULT res =
		Renderer::getDevice()->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logError(res, "Failed creating vertex buffer in Terrain::SubGrid class!\n");
}

unsigned int Terrain::SubGrid::getVerticeCount() const { return m_vertices.size(); }

void Terrain::SubGrid::bind() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

float Terrain::SubGrid::getHeightFromPosition(float x, float z) {
	float X = x;
	float Y = z;
	if (X >= 0 && X < 1 && Y >= 0 && Y < 1) {
		float fx = X * m_heightmapSize.x;
		float fy = Y * m_heightmapSize.y;
		int ix = fx, iy = fy;			  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = float3((ix + 0) / m_heightmapSize.x, m_heightmap[ix + 0][iy + 0],
			(iy + 0) / m_heightmapSize.y);
		float3 p2 = float3((ix + 1) / m_heightmapSize.x, m_heightmap[ix + 1][iy + 0],
			(iy + 0) / m_heightmapSize.y);
		float3 p3 = float3((ix + 0) / m_heightmapSize.x, m_heightmap[ix + 0][iy + 1],
			(iy + 1) / m_heightmapSize.y);
		float3 p4 = float3((ix + 1) / m_heightmapSize.x, m_heightmap[ix + 1][iy + 1],
			(iy + 1) / m_heightmapSize.y);
		if (rx < ry) {
			float3 pt = p3 + 2 * ((float3)((p1 + p4) / 2) - p3);
			float3 p12 = p1 + (pt - p1) * (fx - ix);
			float3 p34 = p3 + (p4 - p3) * (fx - ix);
			float3 p1234 = p12 + (p34 - p12) * (fy - iy);
			return p1234.y;
		}
		else {
			float3 pt = p2 + 2 * ((float3)((p1 + p4) / 2) - p2);
			float3 p12 = p1 + (p2 - p1) * (fx - ix);
			float3 p34 = pt + (p4 - pt) * (fx - ix);
			float3 p1234 = p12 + (p34 - p12) * (fy - iy);
			return p1234.y;
		}
	}
	else {
		return 0;
	}
	return 0;
}

float3 Terrain::SubGrid::getNormalFromPosition(float x, float z) {
	float X = x;
	float Y = z;
	if (X >= 0 && X < 1 && Y >= 0 && Y < 1) {
		float fx = X * m_heightmapSize.x;
		float fy = Y * m_heightmapSize.y;
		int ix = fx, iy = fy;			  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = float3((ix + 0) / m_heightmapSize.x, m_heightmap[ix + 0][iy + 0],
			(iy + 0) / m_heightmapSize.y);
		float3 p2 = float3((ix + 1) / m_heightmapSize.x, m_heightmap[ix + 1][iy + 0],
			(iy + 0) / m_heightmapSize.y);
		float3 p3 = float3((ix + 0) / m_heightmapSize.x, m_heightmap[ix + 0][iy + 1],
			(iy + 1) / m_heightmapSize.y);
		float3 p4 = float3((ix + 1) / m_heightmapSize.x, m_heightmap[ix + 1][iy + 1],
			(iy + 1) / m_heightmapSize.y);
		if (rx < ry) {
			float3 n = (p4 - p3).Cross(p1 - p3);
			n.Normalize();
			return n;
		}
		else {
			float3 n = (p1 - p2).Cross(p4 - p2);
			n.Normalize();
			return n;
		}
	}
	else {
		return float3(0, 0, 0); // outside terrain
	}
}

void Terrain::SubGrid::createMeshFromHeightmap() {
	if (!(m_heightmapSize.x == 0 || m_heightmapSize.y == 0)) {
		m_vertices.reserve((m_heightmapSize.x - 1) * (m_heightmapSize.y - 1) * 6.0);
		// set all positions
		XMINT2 order[6] = { // tri1
			XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
			// tri2
			XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
		};
		for (int yy = 0; yy < m_heightmapSize.y-1; yy++) {
			for (int xx = 0; xx < m_heightmapSize.x-1; xx++) {
				// create triangles
				for (int i = 0; i < 6; i++) {
					XMINT2 index(xx+order[i].x, yy+order[i].y);
					float2 uv = float2((float)index.x / (m_heightmapSize.x - 1),
						(float)index.y / (m_heightmapSize.y - 1));
					m_vertices.push_back(Vertex(float3(uv.x, m_heightmap[index.x][index.y], uv.y),
						float2(uv.x, uv.y), m_heightmapNormal[index.x][index.y]));
				}
			}
		}
		if (0) {
			// fix normals to flat shading
			for (int i = 0; i < m_vertices.size(); i += 3) {
				float3 p1 = m_vertices[i + 0.].position;
				float3 p2 = m_vertices[i + 1.].position;
				float3 p3 = m_vertices[i + 2.].position;
				float3 normal = (p2 - p1).Cross(p3 - p1);
				normal.Normalize();
				m_vertices[i + 0.].normal = normal;
				m_vertices[i + 1.].normal = normal;
				m_vertices[i + 2.].normal = normal;
			}
		}
		createBuffers();
	}
	else {
		// hieghtmap size is not acceptable
	}
}

Terrain::SubGrid::SubGrid() {}

Terrain::SubGrid::~SubGrid() {}
