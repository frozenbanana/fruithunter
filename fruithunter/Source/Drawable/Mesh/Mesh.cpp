#include "Mesh.h"
#include "Renderer.h"
#include "ErrorLogger.h"

ShaderSet Mesh::m_shaderObject;
ShaderSet Mesh::m_shaderObject_onlyMesh;
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::m_colorBuffer;

float Mesh::triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2) {
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

Mesh::BoxIntersection Mesh::obbTest(
	float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale, float& t) {
	// SLABS CALULATIONS(my own)
	float4 data[3] = { float4(1, 0, 0, boxScale.x), float4(0, 1, 0, boxScale.y),
		float4(0, 0, 1, boxScale.z) }; //{o.u_hu,o.v_hv,o.w_hw};
	float Tmin = -9999999999.f, Tmax = 9999999999.f;
	for (size_t i = 0; i < 3; i++) {
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
	}
	if (Tmin < Tmax) {
		t = Tmin;
		if (Tmin < 0 && Tmax > 0)
			return BoxIntersection::InsideHit;
		return BoxIntersection::OutsideHit;
	}
	else
		return BoxIntersection::OutsideMiss;
}
bool Mesh::findMinMaxValues() {
	if (m_meshVertices.size() > 0) {
		float3 boundMin, boundMax;
		for (size_t i = 0; i < m_meshVertices.size(); i++) {
			float3 p = m_meshVertices[i].position;
			if (i == 0) {
				boundMin = p;
				boundMax = p;
			}
			else {
				boundMin.x = min(boundMin.x, p.x);
				boundMin.y = min(boundMin.y, p.y);
				boundMin.z = min(boundMin.z, p.z);

				boundMax.x = max(boundMax.x, p.x);
				boundMax.y = max(boundMax.y, p.y);
				boundMax.z = max(boundMax.z, p.z);
			}
		}
		m_minmaxChanged = true;
		m_boundingBox = CubeBoundingBox(boundMin, boundMax - boundMin);
		return true;
	}
	else
		return false;
}

void Mesh::fillOctree(const CubeBoundingBox& boundingBox, const vector<Vertex>& vertices) {
	size_t triangleCount = vertices.size() / 3;
	m_octree_triangles.initilize(boundingBox, 5, triangleCount);
	for (size_t i = 0; i < triangleCount; i++) {
		Triangle tri;
		tri.vertices[0] = vertices[i * 3 + 0];
		tri.vertices[1] = vertices[i * 3 + 1];
		tri.vertices[2] = vertices[i * 3 + 2];
		vector<float3> points = { tri.vertices[0].position, tri.vertices[1].position,
			tri.vertices[2].position };
		CubeBoundingBox bb(points);
		m_octree_triangles.add(bb, tri, false);
	}
}

void Mesh::updateBoundingBoxBuffer() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	if (m_boxVertices.size() > 0) {
		if (m_minmaxChanged) {
			m_minmaxChanged = false;
			float3 bbPos = getBoundingBoxPos();
			float3 bbScale = getBoundingBoxHalfSizes();
			std::vector<Vertex> fullBox;
			fullBox.reserve(36);
			for (size_t i = 0; i < 36; i++) {
				Vertex v = m_boxVertices[i];
				v.position = v.position * bbScale + bbPos;
				fullBox.push_back(v);
			}
			deviceContext->UpdateSubresource(
				m_vertexBuffer_BoundingBox.Get(), 0, 0, fullBox.data(), 0, 0);
		}
	}
}
void Mesh::loadBoundingBox() {
	// vertices
	m_boxVertices.reserve(36);
	float2 pre[4] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
	for (size_t f = 0; f < 3; f++) // per slab .3
	{
		for (size_t j = 0; j < 2; j++) // parallel planes .2
		{
			size_t m = j * 2 - 1;
			float mf = (float)m;
			Vertex v[4];
			for (size_t i = 0; i < 4; i++) // per point
			{
				float2 a = pre[i];
				float3 p, n;
				if (f == 0)
					p = float3(a.y, a.x, 0) + float3(0, 0, mf), n = float3(0, 0, mf);
				else if (f == 1)
					p = float3(a.x, 0, a.y) + float3(0, mf, 0), n = float3(0, mf, 0);
				else if (f == 2)
					p = float3(0, a.y, a.x) + float3(mf, 0, 0), n = float3(mf, 0, 0);
				float2 uv(a.x, -a.y);
				v[i] = Vertex(p, uv, n);
			}
			if (m == 1)
				m_boxVertices.push_back(v[0]), m_boxVertices.push_back(v[1]),
					m_boxVertices.push_back(v[3]), m_boxVertices.push_back(v[0]),
					m_boxVertices.push_back(v[3]), m_boxVertices.push_back(v[2]);
			else
				m_boxVertices.push_back(v[1]), m_boxVertices.push_back(v[0]),
					m_boxVertices.push_back(v[3]), m_boxVertices.push_back(v[2]),
					m_boxVertices.push_back(v[3]), m_boxVertices.push_back(v[0]);
		}
	}
}
void Mesh::createBuffers(bool instancing) {
	ID3D11Device* gDevice = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	HRESULT check;

	// vertex buffer
	m_vertexBuffer.Reset();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = (UINT)m_meshVertices.size() * sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = m_meshVertices.data();

	check = gDevice->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
	if (FAILED(check))
		ErrorLogger::logError("Failed creating vertex buffer in Mesh class!\n", check);

	// boundingbox buffer
	if (m_vertexBuffer_BoundingBox.Get() == nullptr) {
		D3D11_BUFFER_DESC buffDesc;
		memset(&buffDesc, 0, sizeof(buffDesc));

		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		buffDesc.ByteWidth = sizeof(Vertex) * 36;

		check =
			gDevice->CreateBuffer(&buffDesc, nullptr, m_vertexBuffer_BoundingBox.GetAddressOf());
		if (FAILED(check))
			ErrorLogger::logError("Failed creating boundingbox buffer in Mesh class!\n", check);
	}
	// color buffer
	if (m_colorBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(float4);

		HRESULT res = gDevice->CreateBuffer(&desc, nullptr, m_colorBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError("Failed creating color buffer in Mesh class!\n", res);
	}
}
void Mesh::bindMaterial(int index) { m_materials[m_currentMaterial][index].bind(); }
int Mesh::getVertexCount() const { return (int)m_meshVertices.size(); }
const std::vector<Vertex>& Mesh::getVertexPoints() const { return m_meshVertices; }
const Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::getVertexBuffer() const { return m_vertexBuffer; }
std::string Mesh::getName() const { return m_loadedMeshName; }

void Mesh::setMaterialIndex(int material) { m_currentMaterial = material; }

void Mesh::bindColorBuffer(float4 color) {
	auto deviceContext = Renderer::getDeviceContext();
	// update color buffer
	float4 data = color;
	deviceContext->UpdateSubresource(m_colorBuffer.Get(), 0, 0, &data, 0, 0);
	deviceContext->PSSetConstantBuffers(COLOR_SLOT, 1, m_colorBuffer.GetAddressOf());
}

void Mesh::drawCall_all() { Renderer::draw((UINT)m_meshVertices.size(), 0); }

void Mesh::drawCall_perMaterial() {
	for (size_t i = 0; i < m_parts.size(); i++) {
		for (size_t j = 0; j < m_parts[i].materialUsage.size(); j++) {
			int materialIndex = m_parts[i].materialUsage[j].materialIndex;
			if (materialIndex != -1 && m_currentMaterial >= 0 &&
				m_currentMaterial < m_materials.size()) {
				m_materials[m_currentMaterial][materialIndex].bind(MATERIAL_SLOT);
				int count = m_parts[i].materialUsage[j].count;
				int index = m_parts[i].materialUsage[j].index;
				Renderer::draw(count, index);
			}
		}
	}
}

void Mesh::draw(float3 color, float alpha) {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	if (m_materials[m_currentMaterial].size() > 0) {
		m_shaderObject.bindShadersAndLayout();
		bindColorBuffer(float4(color.x, color.y, color.z, alpha));
		bindMesh();
		drawCall_perMaterial();
	}
	else
		draw_noMaterial(color);
}

void Mesh::draw_noMaterial(float3 color, float alpha) {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	m_shaderObject_onlyMesh.bindShadersAndLayout();
	bindMesh();
	bindColorBuffer(float4(color.x, color.y, color.z, alpha));
	drawCall_all();
}

void Mesh::draw_BoundingBox() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	updateBoundingBoxBuffer();
	m_shaderObject_onlyMesh.bindShadersAndLayout();
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(
		0, 1, m_vertexBuffer_BoundingBox.GetAddressOf(), &strides, &offset);
	Renderer::draw(36, 0);
}
void Mesh::bindMesh() const {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}
float3 Mesh::getBoundingBoxPos() const { return m_boundingBox.getCenter(); }
float3 Mesh::getBoundingBoxSize() const { return m_boundingBox.m_size; }

bool Mesh::load(std::string filename, bool combineParts) {
	if (m_handler.load(filename, m_meshVertices, m_parts, m_materials[0], combineParts)) {
		m_loadedMeshName = filename;
		findMinMaxValues();
		fillOctree(m_boundingBox, m_meshVertices);
		createBuffers();
		return true;
	}
	else
		return false;
}

void Mesh::loadOtherMaterials(std::vector<string> fileNames) {
	if (m_materials.size() < 2) {
		m_materials.resize(fileNames.size()); // Maybe not reload the first one and use +1 for index
		for (size_t i = 0; i < fileNames.size(); ++i) {
			m_materials[i].clear();
			MeshHandler::loadMTL(fileNames[i], m_materials[i]);
		}
	}
}

float Mesh::castRayOnMesh(float3 rayPos, float3 rayDir) {
	if (rayDir.LengthSquared() == 0)
		return -1;
	// get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxHalfSizes();
	// check if close
	float bbt;
	if (obbTest(rayDir, rayPos, bPos, bScale, bbt)) {
		// find the exact point
		vector<Triangle*> triangles;
		m_octree_triangles.cullElements(rayPos, rayDir, triangles);
		float closest = -1;
		for (size_t i = 0; i < triangles.size(); i++) {
			float3 tri[3] = { triangles[i]->vertices[0].position,
				triangles[i]->vertices[1].position, triangles[i]->vertices[2].position };
			float t = triangleTest(rayDir, rayPos, tri[0], tri[1], tri[2]);
			if (t >= 0 && (closest < 0 || t < closest))
				closest = t;
		}
		return closest;

		/*float closest = -1;
		int length = (int)m_meshVertices.size() / 3;
		for (int i = 0; i < length; i++) {
			int index = i * 3;
			float3 v0 = m_meshVertices[(int)index + (int)0].position;
			float3 v1 = m_meshVertices[(int)index + (int)1].position;
			float3 v2 = m_meshVertices[(int)index + (int)2].position;
			float t = triangleTest(rayDir, rayPos, v0, v1, v2);
			if (t >= 0 && (closest < 0 || t < closest))
				closest = t;
		}
		return closest;*/
	}
	return -1;
}

bool Mesh::castRayOnMeshEx(float3 rayPos, float3 rayDir, float3& intersection, float3& normal) {
	if (rayDir.LengthSquared() == 0)
		return false;
	// get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxHalfSizes();
	// check if close
	float bbt;
	if (obbTest(rayDir, rayPos, bPos, bScale, bbt)) {
		// find the exact point
		vector<Triangle*> triangles;
		m_octree_triangles.cullElements(rayPos, rayDir, triangles);
		float3 closest_normal;
		float closest = -1;
		for (size_t i = 0; i < triangles.size(); i++) {
			float3 tri[3] = { triangles[i]->vertices[0].position,
				triangles[i]->vertices[1].position, triangles[i]->vertices[2].position };
			float t = triangleTest(rayDir, rayPos, tri[0], tri[1], tri[2]);
			if (t >= 0 && (closest < 0 || t < closest)) {
				closest = t;
				closest_normal = ((tri[1] - tri[0]).Cross(tri[2] - tri[0]));
			}
		}

		/*float3 closest_normal;
		float closest = -1;
		int length = (int)m_meshVertices.size() / 3;
		for (int i = 0; i < length; i++) {
			int index = i * 3;
			float3 v0 = m_meshVertices[(int)index + (int)0].position;
			float3 v1 = m_meshVertices[(int)index + (int)1].position;
			float3 v2 = m_meshVertices[(int)index + (int)2].position;
			float t = triangleTest(rayDir, rayPos, v0, v1, v2);
			if (t >= 0 && (closest < 0 || t < closest)) {
				closest = t;
				closest_normal = ((v1 - v0).Cross(v2 - v0));
			}
		}*/
		if (closest >= 0) {
			intersection = rayPos + rayDir * closest;
			normal = closest_normal;
			return true;
		}
		else
			return false;
	}
	return false;
}

bool Mesh::castRayOnMeshEx_limitDistance(
	float3 rayPos, float3 rayDir, float3& intersection, float3& normal) {
	if (rayDir.LengthSquared() == 0)
		return false;
	float length = rayDir.Length();
	float3 rayDirNormalized = Normalize(rayDir);
	// get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxHalfSizes();
	// check if close
	float bbt = 0;
	Mesh::BoxIntersection bInter = obbTest(rayDirNormalized, rayPos, bPos, bScale, bbt);
	if (bInter == Mesh::BoxIntersection::InsideHit ||
		(bInter == Mesh::BoxIntersection::OutsideHit && bbt > 0 && bbt < length)) {
		// find the exact point
		float3 closest_normal;
		float closest = -1;
		vector<Triangle*> triangles;
		m_octree_triangles.cullElements_limitDistance(rayPos, rayDir, triangles);
		for (size_t i = 0; i < triangles.size(); i++) {
			float3 tri[3] = { triangles[i]->vertices[0].position,
				triangles[i]->vertices[1].position, triangles[i]->vertices[2].position };
			float t = triangleTest(rayDirNormalized, rayPos, tri[0], tri[1], tri[2]);
			if ((t >= 0 && t < length) && (closest < 0 || t < closest)) {
				closest = t;
				closest_normal = ((tri[1] - tri[0]).Cross(tri[2] - tri[0]));
			}
		}

		// float3 closest_normal;
		// float closest = -1;
		// int length = (int)m_meshVertices.size() / 3;
		// for (int i = 0; i < length; i++) {
		//	int index = i * 3;
		//	float3 v0 = m_meshVertices[(int)index + (int)0].position;
		//	float3 v1 = m_meshVertices[(int)index + (int)1].position;
		//	float3 v2 = m_meshVertices[(int)index + (int)2].position;
		//	float t = triangleTest(rayDirNormalized, rayPos, v0, v1, v2);
		//	if ((t >= 0 && t < length) && (closest < 0 || t < closest)) {
		//		closest = t;
		//		closest_normal = ((v1 - v0).Cross(v2 - v0));
		//	}
		//}

		if (closest >= 0) {
			intersection = rayPos + rayDirNormalized * closest;
			normal = closest_normal;
			return true;
		}
		else
			return false;
	}
	return false;
}

float3 Mesh::getBoundingBoxHalfSizes() const { return m_boundingBox.m_size / 2.f; }


Mesh::Mesh(std::string OBJFile) {
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
	if (!m_shaderObject_onlyMesh.isLoaded())
		m_shaderObject_onlyMesh.createShaders(L"VertexShader_model_onlyMesh.hlsl", nullptr,
			L"PixelShader_model_onlyMesh.hlsl", inputLayout_onlyMesh, 3);
	if (!m_shaderObject.isLoaded())
		m_shaderObject.createShaders(L"VertexShader_model_onlyMesh.hlsl", nullptr,
			L"PixelShader_model.hlsl", inputLayout_onlyMesh, 3);

	m_currentMaterial = 0;
	m_materials.resize(1);
	if (m_boxVertices.size() == 0) {
		loadBoundingBox();
	}
	if (OBJFile != "")
		load(OBJFile);
}
// Need to update
// Mesh & Mesh::operator=(const Mesh & other)
//{
//	minmaxChanged = other.minmaxChanged;
//	MinMaxXPosition = other.MinMaxXPosition;
//	MinMaxYPosition = other.MinMaxYPosition;
//	MinMaxZPosition = other.MinMaxZPosition;
//	parts = other.parts;
//	mesh = other.mesh;
//	materials = other.materials;
//
//	findMinMaxValues();
//	createBuffers();
//
//	return *this;
//}
Mesh::~Mesh() {}
