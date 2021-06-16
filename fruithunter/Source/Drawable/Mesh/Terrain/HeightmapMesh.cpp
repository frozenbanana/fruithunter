#include "HeightmapMesh.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include <WICTextureLoader.h>

float HeightmapMesh::sampleHeightmap(
	float2 uv, const D3D11_MAPPED_SUBRESOURCE& data, const D3D11_TEXTURE2D_DESC& description) { 

	int texWidth = description.Width;
	int texHeight = description.Height;

	XMINT2 iUV((int)(uv.x * (float)(texWidth - 1)), (int)(uv.y * (float)(texHeight - 1)));

	float v = 0;
	if (description.Format == DXGI_FORMAT_R8_UNORM) {
		unsigned char r = ((unsigned char*)data.pData)[iUV.y * data.RowPitch + iUV.x];
		v = (float)r / (pow(2.f, 1.f * 8.f) - 1.f);
	}
	else if (description.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
		unsigned char r =
			((unsigned char*)data.pData)[iUV.y * data.RowPitch + iUV.x * 4];
		v = (float)r / (pow(2.f, 1.f * 8.f) - 1.f);
	}
	else if (description.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
		unsigned char r =
			((unsigned char*)data.pData)[iUV.y * data.RowPitch + iUV.x * 4];
		v = (float)r / 255.f;
	}
	else if (description.Format == DXGI_FORMAT_R16G16B16A16_UNORM) {
		unsigned short int r =
			((unsigned short int*)
					data.pData)[(size_t)iUV.y * (data.RowPitch / sizeof(short int)) + (size_t)iUV.x * 4];
		v = (float)r / (pow(2.f, sizeof(short int) * 8.f) - 1.f);
	}
	return v;
}

void HeightmapMesh::createGridPointBase(XMINT2 gridSize) {
	// create map for points
	m_gridPointSize = gridSize;
	m_gridPoints.resize(m_gridPointSize.x);
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		m_gridPoints[xx].resize(m_gridPointSize.y);
	}
	// map positions and uv
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		for (int yy = 0; yy < m_gridPointSize.y; yy++) {
			float2 uv =
				float2((float)xx / (m_gridPointSize.x - 1), (float)yy / (m_gridPointSize.y - 1));
			m_gridPoints[xx][yy].position = float3(uv.x, 0, uv.y);
			m_gridPoints[xx][yy].uv = uv;
		}
	}
}

bool HeightmapMesh::setGridHeightFromHeightmap(string filename) {
	auto gDevice = Renderer::getDevice();
	auto gDeviceContext = Renderer::getDeviceContext();

	if (filename != "") {
		string path = m_heightmapPath + filename;
		wstring wstr = s2ws(path);
		const wchar_t* fp = wstr.c_str();
		// get texture
		ID3D11Resource* resource = nullptr;
		HRESULT hr = CreateWICTextureFromFileEx(gDevice, fp, 0, D3D11_USAGE_STAGING, 0,
			D3D11_CPU_ACCESS_READ, 0, WIC_LOADER_DEFAULT, &resource, nullptr);
		if (FAILED(hr)) {
			ErrorLogger::logError(
				"[HeightmapMesh::setGridHeightFromHeightmap] Failed loading texture: " + path, hr);
			return false;
		}

		// get mapped data
		D3D11_MAPPED_SUBRESOURCE data;
		hr = gDeviceContext->Map(resource, 0, D3D11_MAP_READ, 0, &data);
		if (FAILED(hr)) {
			ErrorLogger::logError(
				"[HeightmapMesh::setGridHeightFromHeightmap] Failed mapping resource: " + path, hr);
			return false;
		}

		// get description
		D3D11_TEXTURE2D_DESC description;
		ID3D11Texture2D* tex = nullptr;
		hr = resource->QueryInterface(&tex);
		if (FAILED(hr)) {
			ErrorLogger::logError(
				"[HeightmapMesh::setGridHeightFromHeightmap] Failed Texture2D->QueryInterface: " +
					path,
				hr);
			return false;
		}
		tex->GetDesc(&description);
		tex->Release();

		// apply heights
		for (int xx = 0; xx < m_gridPointSize.x; xx++) {
			for (int yy = 0; yy < m_gridPointSize.y; yy++) {
				float3 pos = m_gridPoints[xx][yy].position;
				m_gridPoints[xx][yy].position.y =
					sampleHeightmap(float2(pos.x, pos.z), data, description);
			}
		}

		// remember loaded heightmap filename
		m_previousLoaded = filename;

		// smooth gridpoints
		smoothGrid(SMOOTH_STEPS);

		// unmap data
		gDeviceContext->Unmap(resource, 0);

		return true;
	}
	return false;
}

void HeightmapMesh::smoothGrid(size_t iterations) {
	int smoothSteps = (int)iterations;
	for (int i = 0; i < smoothSteps; i++) {
		vector<vector<Vertex>> mapCopy = m_gridPoints;
		for (size_t xx = 1; xx < (size_t)m_gridPointSize.x - 1; xx++) {
			for (size_t yy = 1; yy < (size_t)m_gridPointSize.y - 1; yy++) {
				float3 current = mapCopy[xx][yy].position;
				float3 average = (mapCopy[xx][yy].position + mapCopy[xx + 1][yy].position +
									 mapCopy[xx][yy + 1].position + mapCopy[xx - 1][yy].position +
									 mapCopy[xx][yy - 1].position) /
								 5.0f;
				m_gridPoints[xx][yy].position = average;
			}
		}
	}
}

void HeightmapMesh::setGridPointNormals() {
	// map normals
	for (size_t xx = 0; xx < (size_t)m_gridPointSize.x - 1; xx++) {
		for (size_t yy = 0; yy < (size_t)m_gridPointSize.y - 1; yy++) {
			float3 points[2][3];
			for (int i = 0; i < 6; i++) {
				XMINT2 index(xx + POINT_ORDER[i].x, yy + POINT_ORDER[i].y);
				points[i / 3][i % 3] = m_gridPoints[index.x][index.y].position;
			}
			float3 normal1 = (points[0][1] - points[0][0]).Cross(points[0][2] - points[0][0]);
			normal1.Normalize();
			float3 normal2 = (points[1][1] - points[1][0]).Cross(points[1][2] - points[1][0]);
			normal2.Normalize();

			m_gridPoints[xx + 1][yy + 1].normal = normal1;
			m_gridPoints[xx + 0][yy + 0].normal = normal1;
			m_gridPoints[xx + 0][yy + 1].normal = normal1;

			m_gridPoints[xx + 0][yy + 0].normal = normal2;
			m_gridPoints[xx + 1][yy + 1].normal = normal2;
			m_gridPoints[xx + 1][yy + 0].normal = normal2;
		}
	}
	// normalize normals
	for (int xx = 0; xx < m_gridPointSize.x; xx++) {
		for (int yy = 0; yy < m_gridPointSize.y - 1; yy++) {
			m_gridPoints[xx][yy].normal.Normalize();
		}
	}
}

float HeightmapMesh::obbTest(float3 rayOrigin, float3 rayDir, float3 boxPos, float3 boxScale) {
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

float HeightmapMesh::triangleTest(
	float3 rayOrigin, float3 rayDir, float3 tri0, float3 tri1, float3 tri2) {

	float3 normal = ((tri1 - tri0).Cross(tri2 - tri0));
	normal.Normalize();
	float3 toTri = rayOrigin - tri0;
	toTri.Normalize();
	float proj = toTri.Dot(normal);
	if (proj < 0)
		return -1;
	// mat3 m = mat3(-rayDir, tri.vtx1xyz - tri.vtx0xyz, tri.vtx2.xyz - tri.vtx0xyz);
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

void HeightmapMesh::tileRayIntersectionTest(
	XMINT2 gridIndex, float3 point, float3 direction, float& minL) {

	size_t ix = gridIndex.x, iy = gridIndex.y;
	// create triangles
	vector<float3> triangles;
	triangles.resize(6);
	for (int i = 0; i < 6; i++) {
		triangles[i] = m_gridPoints[ix + POINT_ORDER[i].x][iy + POINT_ORDER[i].y].position;
	}
	// triangle checks
	float t1 = triangleTest(point, direction, triangles[0], triangles[1], triangles[2]);
	if ((t1 > 0.f) && (minL == -1 || t1 < minL)) {
		minL = t1;
	}
	float t2 = triangleTest(point, direction, triangles[3], triangles[4], triangles[5]);
	if ((t2 > 0.f) && (minL == -1 || t2 < minL)) {
		minL = t2;
	}
}

XMINT2 HeightmapMesh::getSize() const { return m_gridPointSize; }

float HeightmapMesh::getHeightFromUV(float2 uv) { 
	float X = uv.x;
	float Y = uv.y;

	if (X >= 0. && X < 1. && Y >= 0 && Y < 1.) {
		float fx = X * (m_gridPointSize.x - 1);
		float fy = Y * (m_gridPointSize.y - 1);
		size_t ix = (int)fx, iy = (int)fy;	  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = m_gridPoints[ix + 0][iy + 0].position;
		float3 p2 = m_gridPoints[ix + 1][iy + 0].position;
		float3 p3 = m_gridPoints[ix + 0][iy + 1].position;
		float3 p4 = m_gridPoints[ix + 1][iy + 1].position;

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

		return height;
	}
	return 0; // outside terrain
}

float3 HeightmapMesh::getNormalFromUV(float2 uv) { 
	float X = uv.x;
	float Y = uv.y;
	if (X >= 0 && X < 1 && Y >= 0 && Y < 1) {
		float fx = X * (m_gridPointSize.x - 1);
		float fy = Y * (m_gridPointSize.y - 1);
		size_t ix = (int)fx, iy = (int)fy;	  // floor
		float rx = fx - ix, ry = fy - iy; // rest

		float3 p1 = m_gridPoints[ix + 0][iy + 0].position;
		float3 p2 = m_gridPoints[ix + 1][iy + 0].position;
		float3 p3 = m_gridPoints[ix + 0][iy + 1].position;
		float3 p4 = m_gridPoints[ix + 1][iy + 1].position;

		float3 normal;
		if (rx < ry) {
			normal = (p4 - p3).Cross(p1 - p3);
		}
		else {
			normal = (p1 - p2).Cross(p4 - p2);
		}
		normal.Normalize();

		return normal;
	}
	return float3(0, 0, 0); // outside terrain
}

float HeightmapMesh::castRay(float3 startPoint, float3 endPoint) { 
	float3 n = endPoint - startPoint;
	float2 n2 = float2(n.x, n.z);
	float length = n.Length();
	n.Normalize();
	float obb_l = obbTest(startPoint, n, float3(1, 1, 1) * 0.5f, float3(1, 1, 1) * 0.5f);
	if (obb_l > 0) {
		// values in grid coordinates [0,m_gridPointSize.x-1]
		float2 tilt(n2.x * (m_gridPointSize.x - 1), n2.y * (m_gridPointSize.y - 1));
		float2 start(
			startPoint.x * (m_gridPointSize.x - 1), (float)startPoint.z * (m_gridPointSize.y - 1));
		XMINT2 iStart((int)start.x, (int)start.y);
		float2 end(endPoint.x * (m_gridPointSize.x - 1), endPoint.z * (m_gridPointSize.y - 1));
		XMINT2 iEnd((int)end.x, (int)end.y);
		// find intersection tiles
		vector<float> tsX, tsY;
		size_t changeInX = (size_t)abs(iEnd.x - iStart.x);
		size_t changeInY = (size_t)abs(iEnd.y - iStart.y);
		tsX.reserve(changeInX);
		tsY.reserve(changeInY);
		for (int i = 0; i < changeInX; i++) {
			float t = ((iStart.x + (iStart.x > iEnd.x ? -1 * i : 1 * i + 1)) - start.x) / tilt.x;
			tsX.push_back(t);
		}
		for (int i = 0; i < changeInY; i++) {
			float t = ((iStart.y + (iStart.y > iEnd.y ? -1 * i : i * 1 + 1)) - start.y) / tilt.y;
			tsY.push_back(t);
		}
		vector<float> ts;					   // sorted intersection time array
		ts.reserve(changeInX + changeInY + 2); //+2 for start and end point
		// sort largest first
		ts.push_back(1);
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
		float minL = -1;
		for (int i = (int)ts.size() - 2; i >= 0; i--) {
			float sampledT = (ts[i] + ts[(size_t)i + 1]) / 2.f;
			int ix = (int)((float)start.x + tilt.x * sampledT);
			int iy = (int)((float)start.y + tilt.y * sampledT);
			if (ix >= 0 && ix < m_gridPointSize.x - 1 && iy >= 0 && iy < m_gridPointSize.y - 1) {
				tileRayIntersectionTest(XMINT2(ix, iy), startPoint, n, minL);
				if (minL != -1)
					break; // early break
			}
		}
		// return
		if (minL != -1 && minL < length) {
			// minL is between [0,length], needs to be mapped [0,1] to be valid
			float mappedL = minL / length;
			return mappedL; 
		}
	}
	return -1;
}

bool HeightmapMesh::validPosition(float2 point, float4x4 worldMatrix) {
	//check height
	float3 lPoint = float3(point.x, getHeightFromUV(point), point.y);
	float3 wPoint = float3::Transform(lPoint, worldMatrix);
	if (wPoint.y < 1)
		return false;

	//check normal
	float4x4 worldInvTraMatrix = worldMatrix.Invert().Transpose();
	float3 lNormal = getNormalFromUV(point);
	float3 wNormal = Normalize(float3::Transform(lNormal, worldInvTraMatrix));
	if (wNormal.Dot(float3(0, 1, 0)) < 0.7)
		return false;

	return true; // valid position

}

bool HeightmapMesh::containsValidPosition(float2 point, float2 size, float4x4 worldMatrix) { 
	float4x4 worldInvTraMatrix = worldMatrix.Invert().Transpose();
	float2 fsize(float(m_gridPointSize.x - 1), float(m_gridPointSize.y - 1));
	XMINT2 isize(int(fsize.x), int(fsize.y));
	// point1
	float2 fpoint1 = point * fsize;
	XMINT2 ipoint1 = XMINT2((int)floor(fpoint1.x), (int)floor(fpoint1.y));
	ipoint1 = XMINT2(
		Clamp<int>(ipoint1.x, 0, isize.x), Clamp<int>(ipoint1.y, 0, isize.y)); // clamp to grid
	//point2
	float2 fpoint2 = (point + size) * fsize;
	XMINT2 ipoint2 = XMINT2((int)ceil(fpoint2.x), (int)ceil(fpoint2.y));
	ipoint2 = XMINT2(
		Clamp<int>(ipoint2.x, 0, isize.x), Clamp<int>(ipoint2.y, 0, isize.y)); // clamp to grid
	// check vertices between points
	for (size_t xx = ipoint1.x; xx <= ipoint2.x; xx++) {
		for (size_t yy = ipoint1.y; yy < ipoint2.y; yy++) {
			float3 wPosition = float3::Transform(m_gridPoints[xx][yy].position, worldMatrix);
			float3 wNormal = Normalize(float3::Transform(m_gridPoints[xx][yy].normal, worldInvTraMatrix));
			// flat ground and above water
			if (wPosition.y > 1 && wNormal.Dot(float3(0, 1, 0)) > 0.7f)
				return true;
		}
	}
	return false;
}

bool HeightmapMesh::editMesh(const Brush& brush, Brush::Type type, float dt, float4x4 matWorld) {
	float b_wRadius = brush.radius;
	float3 b_wPosition = brush.position;
	float3 b_lPosition = float3::Transform(brush.position, matWorld.Invert());
	bool updated = false;
	for (size_t x = 0; x < m_gridPointSize.x; x++) {
		for (size_t y = 0; y < m_gridPointSize.y; y++) {
			float3 wPoint = float3::Transform(m_gridPoints[x][y].position, matWorld);
			float wDist = (float2(wPoint.x, wPoint.z) - float2(b_wPosition.x, b_wPosition.z)).Length();
			if (wDist < b_wRadius) {
				float effect = 1 - wDist / b_wRadius;
				float smoothedMix =
					1.0f - float(pow(1.f - 0.5f * (1.f - cos(effect * 3.1415f)), 1.f / brush.falloff));
				if (type == Brush::Raise) {
					m_gridPoints[x][y].position.y = Clamp<float>(
						m_gridPoints[x][y].position.y + smoothedMix * brush.strength * dt, 0, 1);
				}
				else if (type == Brush::Lower) {
					m_gridPoints[x][y].position.y = Clamp<float>(
						m_gridPoints[x][y].position.y - smoothedMix * brush.strength * dt, 0, 1);
				}
				else if (type == Brush::Flatten) {
					m_gridPoints[x][y].position.y =
						Clamp<float>(m_gridPoints[x][y].position.y +
										 (b_lPosition.y - m_gridPoints[x][y].position.y) *
											 smoothedMix * brush.strength * 20.f * dt,
							0, 1);
				}
				updated = true;
			}
		}
	}
	if (updated) {
		setGridPointNormals();
		return true;
	}
	return false;
}

void HeightmapMesh::editMesh_push() {
	if (m_gridPointSize.x == 0 || m_gridPointSize.y == 0) {
		// do nothing (invalid size to push to stack)
	}
	else {
		shared_ptr<vector<vector<float>>> meshSample = make_shared<vector<vector<float>>>();
		meshSample->resize(m_gridPointSize.x);
		for (size_t x = 0; x < m_gridPointSize.x; x++) {
			meshSample->at(x).resize(m_gridPointSize.y);
			for (size_t y = 0; y < m_gridPointSize.y; y++) {
				meshSample->at(x)[y] = m_gridPoints[x][y].position.y;
			}
		}
		// add to stack
		m_editMesh_stack.push_back(meshSample);
		if (m_editMesh_stack.size() > EDITMESH_STACKSIZE)
			m_editMesh_stack.erase(m_editMesh_stack.begin()); // remove first if to many samples
	}
}

bool HeightmapMesh::editMesh_pop() {
	if (m_editMesh_stack.size() > 0) {
		// get latest sample
		shared_ptr<vector<vector<float>>> sample = m_editMesh_stack.back();
		m_editMesh_stack.pop_back();
		// recreate from sample
		if (sample->size() == m_gridPointSize.x && sample->at(0).size() == m_gridPointSize.y) {
			// set heights
			for (size_t x = 0; x < m_gridPointSize.x; x++) {
				for (size_t y = 0; y < m_gridPointSize.y; y++) {
					m_gridPoints[x][y].position.y = sample->at(x)[y];
				}
			}
			// reset normal
			setGridPointNormals();

			return true;
		}
		else {
			// invalid sample size (doesnt match size)
			return false;
		}
	}
	return false;
}

void HeightmapMesh::editMesh_clear() { m_editMesh_stack.clear(); }

void HeightmapMesh::loadFromFile_binary(fstream& file) {
	// gridPoint size
	file.read((char*)&m_gridPointSize, sizeof(XMINT2)); // add when application is stable!
	// create mem and align position and uvs (normals and y axis not set)
	createGridPointBase(m_gridPointSize);
	// set point heights
	float* ypositions = new float[(size_t)m_gridPointSize.x * m_gridPointSize.y];
	file.read((char*)ypositions, sizeof(float) * m_gridPointSize.x * m_gridPointSize.y);
	size_t index = 0;
	for (size_t x = 0; x < m_gridPointSize.x; x++)
		for (size_t y = 0; y < m_gridPointSize.y; y++)
			m_gridPoints[x][y].position.y = ypositions[index++]; // set y axis
	delete[] ypositions;
	// set normals
	setGridPointNormals();
}

void HeightmapMesh::storeToFile_binary(fstream& file) {
	// gridPoint size
	file.write((char*)&m_gridPointSize, sizeof(XMINT2)); // add when application is stable!
	// heightmap points
	for (size_t x = 0; x < m_gridPointSize.x; x++)
		for (size_t y = 0; y < m_gridPointSize.y; y++)
			file.write((char*)&m_gridPoints[x][y].position.y, sizeof(float));
}

void HeightmapMesh::changeSize(XMINT2 gridSize) {
	if (gridSize.x == 0 || gridSize.y == 0)
		return; // invalid size
	
	bool sampleFromGrid = !(m_gridPointSize.x == 0 || m_gridPointSize.y == 0);

	vector<vector<Vertex>> gridPoints;
	gridPoints.resize(gridSize.x);
	for (int xx = 0; xx < gridSize.x; xx++) {
		gridPoints[xx].resize(gridSize.y);

		for (int yy = 0; yy < gridSize.y; yy++) {
			float2 uv = float2((float)xx / (gridSize.x - 1), (float)yy / (gridSize.y - 1));
			gridPoints[xx][yy].position =
				float3(uv.x, sampleFromGrid ? getHeightFromUV(uv) : 0, uv.y);
			gridPoints[xx][yy].uv = uv;
		}
	}

	m_gridPointSize = gridSize;
	m_gridPoints = gridPoints;

	setGridPointNormals();
}

void HeightmapMesh::init(string filename, XMINT2 gridSize) {
	// create base
	createGridPointBase(gridSize);
	// set 
	setGridHeightFromHeightmap(filename);
}

vector<Vertex>& HeightmapMesh::operator[](const size_t& index) { return m_gridPoints[index]; }

Vertex& HeightmapMesh::operator[](const XMINT2& index) { return m_gridPoints[index.x][index.y]; }

HeightmapMesh& HeightmapMesh::operator=(const HeightmapMesh& other) {
	m_previousLoaded = other.m_previousLoaded;
	m_gridPointSize = other.m_gridPointSize;
	m_gridPoints = other.m_gridPoints;

	vector<shared_ptr<vector<vector<float>>>> m_editMesh_stack;

	m_editMesh_stack.clear();
	m_editMesh_stack.resize(other.m_editMesh_stack.size());
	for (size_t i = 0; i < other.m_editMesh_stack.size(); i++)
		m_editMesh_stack[i] = make_shared<vector<vector<float>>>(*other.m_editMesh_stack[i].get());

	return *this;
}

HeightmapMesh::HeightmapMesh() {}
