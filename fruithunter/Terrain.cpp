#include "Terrain.hpp"

ShaderSet Terrain::m_shader;
Microsoft::WRL::ComPtr<ID3D11Buffer> Terrain::m_matrixBuffer;

void Terrain::createBuffers() {
	auto gDevice = Renderer::getDevice();
	auto gDeviceContext = Renderer::getDeviceContext();
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
	// grass texture
	createResourceBuffer(m_grassPath, m_map_grass.GetAddressOf());
}

float4x4 Terrain::getModelMatrix() {
	float4x4 transform = float4x4::CreateTranslation(m_position);
	float4x4 rotation = float4x4::CreateRotationY(m_rotation.y);
	float4x4 scale = float4x4::CreateScale(m_scale);
	return scale * rotation * transform;
}

void Terrain::bindModelMatrix() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	float4x4 mWorld = getModelMatrix();
	ModelBuffer matrix;
	matrix.mWorld = mWorld.Transpose();
	matrix.mWorldInvTra =
		mWorld.Invert(); // double transposes does nothing, so the transposes are removed!
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, 0, &matrix, 0, 0);

	deviceContext->VSSetConstantBuffers(MATRIX_BUFFER_SLOT, 1, m_matrixBuffer.GetAddressOf());
}

bool Terrain::loadHeightmap(string filePath) {
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
		return false;
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
				v = (float)d / (pow(2.f, 1.f * 8.f) - 1.f);
			}
			else if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix * 4];
				v = (float)d / (pow(2.f, 1.f * 8.f) - 1.f);
			}
			else if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix * 4];
				v = (float)d / (pow(2.f, 1.f * 8.f) - 1.f);
			}
			else if (texD.Format == DXGI_FORMAT_R16G16B16A16_UNORM) {
				unsigned short int d =
					((unsigned short int*)
							sub.pData)[iy * (sub.RowPitch / sizeof(short int)) + ix * 4];
				v = (float)d / (pow(2.f, sizeof(short int) * 8.f) - 1.f);
			}
			m_heightmap[xx][yy] = v;
		}
	}
	for (int xx = 0; xx < m_heightmapSize.x; xx++) {
		for (int yy = 0; yy < m_heightmapSize.y; yy++) {
			m_heightmapNormals[xx][yy] = calcNormalFromHeightmap(XMINT2(xx, yy));
		}
	}
	return true;
}

float Terrain::sampleHeightmap(float2 uv) {
	XMINT2 index = XMINT2((uint32_t)round(uv.x * (m_heightmapSize.x - 1)),
		(uint32_t)round(uv.y * (m_heightmapSize.y - 1)));
	return m_heightmap[index.x][index.y];
}

float3 Terrain::calcNormalFromHeightmap(XMINT2 index) {
	bool minX = (index.x <= 0);
	bool maxX = (index.x >= m_heightmapSize.x - 1);
	bool minY = (index.y <= 0);
	bool maxY = (index.y >= m_heightmapSize.y - 1);

	float2 pos(1.0f / m_heightmapSize.x, 1.0f / (float)m_heightmapSize.y);
	float3 p = float3(0, (float)m_heightmap[index.x][index.y], 0);
	float3 pTop = float3(0, minY ? 0 : (float)m_heightmap[index.x][index.y - 1], -pos.y);
	float3 pRight = float3(pos.x, maxX ? 0 : (float)m_heightmap[index.x + 1][index.y], 0);
	float3 pLeft = float3(-pos.x, minX ? 0 : (float)m_heightmap[index.x - 1][index.y], 0);
	float3 pBottom = float3(0, maxY ? 0 : (float)m_heightmap[index.x][index.y + 1], pos.y);

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
	XMINT2 index = XMINT2((int32_t)round(uv.x * (m_heightmapSize.x - 1)),
		(int32_t)round(uv.y * (m_heightmapSize.y - 1)));
	return m_heightmapNormals[index.x][index.y];
}

void Terrain::createGridPointsFromHeightmap() {
	XMINT2 order[6] = { // tri1
		XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
		// tri2
		XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
	};

	// create map for points
	m_gridPointSize = XMINT2(m_tileSize.x * m_gridSize.x + 1, m_tileSize.y * m_gridSize.y + 1);
	m_gridPoints.resize(m_gridPointSize.x);
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		m_gridPoints[xx].resize(m_gridPointSize.y);
	}
	// map positions and uv
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		for (int yy = 0; yy < m_gridPointSize.y; yy++) {
			float2 uv =
				float2((float)xx / (m_gridPointSize.x - 1), (float)yy / (m_gridPointSize.y - 1));
			m_gridPoints[xx][yy].position = float3(uv.x, sampleHeightmap(uv), uv.y);
			m_gridPoints[xx][yy].uv = uv;
		}
	}
	// smooth positions
	int smoothSteps = 2;
	for (int i = 0; i < smoothSteps; i++) {
		vector<vector<Vertex>> mapCopy = m_gridPoints;
		for (int xx = 1; xx < m_gridPointSize.x - 1; xx++) {
			for (int yy = 1; yy < m_gridPointSize.y - 1; yy++) {
				float3 current = mapCopy[xx][yy].position;
				float3 average = (mapCopy[xx][yy].position + mapCopy[xx + 1.][yy].position +
									 mapCopy[xx][yy + 1.].position + mapCopy[xx - 1.][yy].position +
									 mapCopy[xx][yy - 1.].position) /
								 5.0f;
				m_gridPoints[xx][yy].position = average;
			}
		}
	}
	// map normals
	for (int xx = 0; xx < m_gridPointSize.x - 1; xx++) {
		for (int yy = 0; yy < m_gridPointSize.y - 1; yy++) {
			float3 points[2][3];
			for (int i = 0; i < 6; i++) {
				XMINT2 index(xx + order[i].x, yy + order[i].y);
				points[i / 3][i % 3] = m_gridPoints[index.x][index.y].position;
			}
			float3 normal1 = (points[0][1] - points[0][0]).Cross(points[0][2] - points[0][0]);
			normal1.Normalize();
			float3 normal2 = (points[1][1] - points[1][0]).Cross(points[1][2] - points[1][0]);
			normal2.Normalize();

			m_gridPoints[xx + 1.][yy + 1.].normal = normal1;
			m_gridPoints[xx + 0.][yy + 0.].normal = normal1;
			m_gridPoints[xx + 0.][yy + 1.].normal = normal1;

			m_gridPoints[xx + 0.][yy + 0.].normal = normal2;
			m_gridPoints[xx + 0.][yy + 0.].normal = normal2;
			m_gridPoints[xx + 1.][yy + 0.].normal = normal2;
		}
	}
	// normalize normals
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		for (int yy = 0; yy < m_gridPointSize.y - 1; yy++) {
			m_gridPoints[xx][yy].normal.Normalize();
		}
	}
}

void Terrain::createGrid(XMINT2 size) {
	if (size.x > 0 && size.y > 0) {
		m_gridSize = size;
		m_subMeshes.resize(m_gridSize.x);
		for (int x = 0; x < m_gridSize.y; x++) {
			m_subMeshes[x].resize(m_gridSize.y);
		}
	}
	else {
		ErrorLogger::logWarning(
			HRESULT(), "WARNING! Tried to change a terrain to inappropriate size");
	}
}

void Terrain::fillSubMeshes(bool flatShaded) {
	if (m_gridPointSize.x != 0 && m_gridPointSize.y != 0) {
		XMINT2 order[6] = { // tri1
			XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
			// tri2
			XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
		};
		for (int ixx = 0; ixx < m_gridSize.x; ixx++) {
			for (int iyy = 0; iyy < m_gridSize.y; iyy++) {
				vector<Vertex>* vertices = m_subMeshes[ixx][iyy].getPtr();
				vertices->clear();
				vertices->reserve((m_tileSize.x) * (m_tileSize.y) * 6);

				XMINT2 indexStart(ixx * m_tileSize.x, iyy * m_tileSize.y);
				XMINT2 indexStop(indexStart.x + m_tileSize.x, indexStart.y + m_tileSize.y);
				for (int xx = indexStart.x; xx < indexStop.x; xx++) {
					for (int yy = indexStart.y; yy < indexStop.y; yy++) {
						for (int i = 0; i < 6; i++) {
							XMINT2 index(xx + order[i].x, yy + order[i].y);
							vertices->push_back(m_gridPoints[index.x][index.y]);
						}
					}
				}
				// flatshade
				if (flatShaded) {
					// fix normals to flat shading
					for (int i = 0; i < vertices->size(); i += 3) {
						float3 p1 = (*vertices)[i + 0.].position;
						float3 p2 = (*vertices)[i + 1.].position;
						float3 p3 = (*vertices)[i + 2.].position;
						float3 normal = (p2 - p1).Cross(p3 - p1);
						normal.Normalize();
						(*vertices)[i + 0.].normal = normal;
						(*vertices)[i + 1.].normal = normal;
						(*vertices)[i + 2.].normal = normal;
					}
				}
				// create buffers
				m_subMeshes[ixx][iyy].initilize();
			}
		}
	}
	else {
		// invalid size
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

	// convert from wide char to narrow char array
	char ch[256];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, str, -1, ch, 256, &DefChar, NULL); // No error checking

	std::string sbuff = std::string(ch);

	return sbuff;
}

bool Terrain::createResourceBuffer(string path, ID3D11ShaderResourceView** buffer) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	wstring wstr = s2ws(path);
	LPCWCHAR str = wstr.c_str();
	HRESULT hrA = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, buffer);
	if (FAILED(hrA)) {
		ErrorLogger::messageBox(hrA, "Failed creating texturebuffer from texture\n" + path);
		return false;
	}
	return true;
}

void Terrain::tileRayIntersectionTest(
	XMINT2 gridIndex, float3 point, float3 direction, float& minL, float3& normal) {
	XMINT2 order[6] = { // tri1
		XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
		// tri2
		XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
	};

	int ix = gridIndex.x, iy = gridIndex.y;
	//create triangles
	vector<float3> triangles;
	triangles.resize(6);
	for (int i = 0; i < 6; i++) {
		triangles[i] = m_gridPoints[ix + order[i].x][iy + order[i].y].position;
	}
	// triangle checks
	float t1 = triangleTest(point, direction, triangles[0], triangles[1], triangles[2]);
	if ((t1 > 0.f) && (minL == -1 || t1 < minL)) {
		minL = t1;
		normal = (triangles[1] - triangles[0]).Cross(triangles[2] - triangles[0]);
	}
	float t2 = triangleTest(point, direction, triangles[3], triangles[4], triangles[5]);
	if ((t2 > 0.f) && (minL == -1 || t2 < minL)) {
		minL = t2;
		normal = (triangles[4] - triangles[3]).Cross(triangles[5] - triangles[0]);
	}
}

float Terrain::obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale) {
	// SLABS CALULATIONS(my own)
	float4 data[3] = { float4(1, 0, 0, boxScale.x), float4(0, 1, 0, boxScale.y),
		float4(0, 0, 1, boxScale.z) }; //{o.u_hu,o.v_hv,o.w_hw};
	float Tmin = -99999999.f, Tmax = 9999999999.f;
	for (int i = 0; i < 3; i++) {
		float3 tempNormal = float3(data[i].x, data[i].y, data[i].z);
		float3 center1 = boxPos + tempNormal * data[i].w;
		float3 center2 = boxPos - tempNormal * data[i].w;
		float npd = tempNormal.Dot(rayDir);
		if (npd != 0) {
			float t1 = (tempNormal.Dot(center1) - tempNormal.Dot(rayOrigin)) / npd;
			float t2 = (tempNormal.Dot(center2) - tempNormal.Dot(rayOrigin)) / npd;
			if (t1 > t2) {
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}
			if (t1 > Tmin) {
				Tmin = t1;
			}
			if (t2 < Tmax)
				Tmax = t2;
		}
		// else return -1;
	}
	if (Tmin < Tmax) {
		if (Tmin < 0)
			return Tmax;
		else
			return Tmin;
	}
	else
		return -1;
}

float Terrain::triangleTest(
	float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2) {

	float3 normal = ((tri1 - tri0).Cross(tri2 - tri0));
	normal.Normalize();
	float3 toTri = rayOrigin - tri0;
	toTri.Normalize();
	float proj = toTri.Dot(normal);
	if (proj < 0)
		return -1;
	// mat3 m = mat3(-rayDir, tri.vtx1.xyz - tri.vtx0.xyz, tri.vtx2.xyz - tri.vtx0.xyz);
	float4x4 m(-rayDir, tri1 - tri0, tri2 - tri0);
	float3 op0 = rayOrigin - tri0;
	float3 tuv = XMVector3Transform(op0, m.Invert());
	float4 tuvw = float4(tuv.x, tuv.y, tuv.z, 0);
	tuvw.w = 1 - tuvw.y - tuvw.z;
	if (tuvw.y >= 0 && tuvw.y <= 1 && tuvw.z >= 0 && tuvw.z <= 1 && tuvw.w >= 0 && tuvw.w <= 1)
		return tuvw.x;
	else
		return -1;
}

void Terrain::initilize(string filename, XMINT2 subsize, XMINT2 splits) {
	if (filename == "" || (splits.x == 0 || splits.y == 0) || (subsize.x == 0 || subsize.y == 0)) {
		// do nothing
	}
	else {
		m_isInitilized = true;
		m_tileSize = subsize;
		createGrid(splits); // create space for memory
		if (loadHeightmap(m_heightmapPath + filename)) {
			createGridPointsFromHeightmap();
			fillSubMeshes();
			// loadGrids();
			createBuffers();
		}
	}
}

void Terrain::rotateY(float radian) {
	m_modelMatrixChanged = true;
	m_rotation.y += radian;
}

void Terrain::setScale(float3 scale) { m_scale = scale; }

float Terrain::getHeightFromPosition(float x, float z) {
	float3 position(x, 0, z);
	float4x4 mTerrainWorld = getModelMatrix();
	float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
	position = float3::Transform(position, mTerrainInvWorld);

	float X = position.x;
	float Y = position.z;

	if (X >= 0 && X < 1 && Y >= 0 && Y < 1) {
		float fx = X * (m_gridPointSize.x - 1);
		float fy = Y * (m_gridPointSize.y - 1);
		int ix = fx, iy = fy;			  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = m_gridPoints[ix + 0.][iy + 0.].position;
		float3 p2 = m_gridPoints[ix + 1.][iy + 0.].position;
		float3 p3 = m_gridPoints[ix + 0.][iy + 1.].position;
		float3 p4 = m_gridPoints[ix + 1.][iy + 1.].position;

		float height;
		if (rx < ry) {
			float3 pt = p3 + 2 * ((float3)((p1 + p4) / 2) - p3);
			float3 p12 = p1 + (pt - p1) * rx;
			float3 p34 = p3 + (p4 - p3) * rx;
			float3 p1234 = p12 + (p34 - p12) * ry;
			height = p1234.y;
		}
		else {
			float3 pt = p2 + 2 * ((float3)((p1 + p4) / 2) - p2);
			float3 p12 = p1 + (p2 - p1) * rx;
			float3 p34 = pt + (p4 - pt) * rx;
			float3 p1234 = p12 + (p34 - p12) * ry;
			height = p1234.y;
		}

		float3 pos(0, height, 0);
		return float3::Transform(pos, mTerrainWorld).y;
	}
	return 0;//outside terrain
}

float3 Terrain::getNormalFromPosition(float x, float z) {
	float3 position(x, 0, z);
	float4x4 mTerrainWorld = getModelMatrix();
	float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
	position = float3::Transform(position, mTerrainInvWorld);

	float X = position.x;
	float Y = position.z;
	if (X >= 0 && X < 1 && Y >= 0 && Y < 1) {
		float fx = X * (m_gridPointSize.x - 1);
		float fy = Y * (m_gridPointSize.y - 1);
		int ix = fx, iy = fy;			  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = m_gridPoints[ix + 0.][iy + 0.].position;
		float3 p2 = m_gridPoints[ix + 1.][iy + 0.].position;
		float3 p3 = m_gridPoints[ix + 0.][iy + 1.].position;
		float3 p4 = m_gridPoints[ix + 1.][iy + 1.].position;

		float3 normal;
		if (rx < ry) {
			normal = (p4 - p3).Cross(p1 - p3);
		}
		else {
			normal = (p1 - p2).Cross(p4 - p2);
		}
		normal.Normalize();

		float3 cn = float3::Transform(normal, mTerrainWorld.Invert().Transpose());
		cn.Normalize();
		return cn;
	}
	return float3(0, 0, 0);//outside terrain
}

bool Terrain::castRay(float3& point, float3& direction) {
	// convert to local space
	float4x4 mTerrainWorld = getModelMatrix();
	float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
	float3 startPoint = float3::Transform(point, mTerrainInvWorld);
	float3 endPoint = float3::Transform(point + direction, mTerrainInvWorld);
	float3 n = endPoint - startPoint;
	float length = n.Length();
	n.Normalize();
	float obb_l = obbTest(startPoint, n, float3(1, 1, 1) * 0.5f, float3(1, 1, 1) * 0.5f);
	if (obb_l > 0) {
		// values in grid coordinates [0,m_gridPointSize.x-1]
		float2 tilt(n.x * (m_gridPointSize.x - 1), n.z * (m_gridPointSize.y - 1));
		float2 start(clamp(startPoint.x * (m_gridPointSize.x - 1), 0, m_gridPointSize.x - 2),
			clamp(startPoint.z * (m_gridPointSize.y - 1), 0, m_gridPointSize.y - 2));
		XMINT2 iStart(start.x, start.y);
		float2 end(clamp(endPoint.x * (m_gridPointSize.x - 1), 0, m_gridPointSize.x - 2),
			clamp(endPoint.z * (m_gridPointSize.y - 1), 0, m_gridPointSize.y - 2));
		XMINT2 iEnd(end.x, end.y);
		// find intersection tiles
		vector<float> tsX, tsY;
		tsX.reserve(abs(iEnd.x - iStart.x));
		tsY.reserve(abs(iEnd.y - iStart.y));
		for (size_t i = 0; i < abs(iEnd.x - iStart.x); i++) {
			float t = ((1 + iStart.x + (iStart.x > iEnd.x ? -1 : 1) * i) - start.x) / tilt.x;
			tsX.push_back(t);
		}
		for (size_t i = 0; i < abs(iEnd.y - iStart.y); i++) {
			float t = ((1 + iStart.y + (iStart.y > iEnd.y ? -1 : 1) * i) - start.y) / tilt.y;
			tsY.push_back(t);
		}
		vector<float> ts; // sorted intersection time array
		ts.reserve(abs(iEnd.x - iStart.x) + abs(iEnd.y - iStart.y) + 1); //+1 for start point
		// sort largest first
		//ts.push_back((end.x - start.x) / tilt.x);
		while (tsX.size() > 0 || tsY.size() > 0) {
			if (tsX.size() > 0 && tsY.size() > 0) {
				if (tsX.back() < tsY.back()) {
					ts.push_back(tsY.back());
					tsY.pop_back();
				}
				else {
					ts.push_back(tsX.back());
					tsX.pop_back();
				}
			}
			else if (tsX.size() > 0) {
				ts.push_back(tsX.back());
				tsX.pop_back();
			}
			else {
				ts.push_back(tsY.back());
				tsY.pop_back();
			}
		}
		ts.push_back(0);
		// check all intersected tiles
		float3 normal;
		float minL = -1;
		for (int i = ts.size() - 1; i >= 0; i--) {
			//float sampledT = (ts[i]+ts[i+1]) / 2.f;
			float sampledT = ts[i];
			int ix = clamp(start.x + tilt.x * sampledT, 0, m_gridPointSize.x - 2);
			int iy = clamp(start.y + tilt.y * sampledT, 0, m_gridPointSize.y - 2);
			tileRayIntersectionTest(XMINT2(ix, iy), startPoint, n, minL, normal);
			if (minL != -1)
				break;//early break
		}
		// convert back to world space
		if (minL != -1) {
			float3 intersectPoint = startPoint + n * minL;
			float3 intersectNormal = normal;
			intersectPoint = float3::Transform(intersectPoint, mTerrainWorld);
			intersectNormal =
				float3::Transform(intersectNormal, mTerrainWorld.Invert().Transpose());
			intersectNormal.Normalize();

			point = intersectPoint;
			direction = intersectNormal;
			return true;
		}
	}
	return false;
}

void Terrain::draw() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	m_shader.bindShadersAndLayout();

	deviceContext->PSSetShaderResources(0, 1, m_map_grass.GetAddressOf());

	bindModelMatrix();

	for (int xx = 0; xx < m_gridSize.x; xx++) {
		for (int yy = 0; yy < m_gridSize.y; yy++) {
			m_subMeshes[xx][yy].bind();
			deviceContext->Draw(m_subMeshes[xx][yy].getVerticeCount(), 0);
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

vector<Vertex>* Terrain::SubGrid::getPtr() { return &m_vertices; }

void Terrain::SubGrid::initilize() { createBuffers(); }

void Terrain::SubGrid::createBuffers() {
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
		ErrorLogger::logError(res, "Failed creating vertex buffer in Terrain::SubGrid class!\n");
}

unsigned int Terrain::SubGrid::getVerticeCount() const { return (unsigned int)m_vertices.size(); }

void Terrain::SubGrid::bind() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Terrain::SubGrid::castRay(float3& point, float3& direction) {
	if (Terrain::obbTest(point, direction, float3(1, 1, 1) * 0.5, float3(1, 1, 1) * 0.5) >= 0) {
		float3 normal;
		float minL = -1;
		for (size_t i = 0; i < m_vertices.size(); i += 3) {
			float3 triangle0 = m_vertices[i + 0].position;
			float3 triangle1 = m_vertices[i + 1].position;
			float3 triangle2 = m_vertices[i + 2].position;
			float l = triangleTest(point, direction, triangle0, triangle1, triangle2);
			if (l >= 0 && (minL == -1 || l < minL)) {
				minL = l;
				normal = (triangle1 - triangle0).Cross(triangle2 - triangle0);
			}
		}
		if (minL >= 0) {
			normal.Normalize();
			float3 p = point, d = direction;
			direction = normal;
			point = p + d * minL;
			return true;
		}
		else {
			// no intersection
			return false;
		}
	}
	else {
		// miss
		return false;
	}
}

Terrain::SubGrid::SubGrid() {}

Terrain::SubGrid::~SubGrid() {}
