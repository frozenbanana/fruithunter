#include "Mesh.hpp"

std::vector<Vertex> Mesh::m_boxVertices;
ShaderSet Mesh::m_shaderObject;
ShaderSet Mesh::m_shaderObject_onlyMesh;
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::m_vertexBuffer_BoundingBox;
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

float Mesh::obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale) {
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
		else
			return -1;
	}
	if (Tmin < Tmax)
		return Tmin;
	else
		return -1;
}
bool Mesh::findMinMaxValues() {
	if (m_meshVertices.size() > 0) {
		DirectX::XMINT2 changedX(1, 1), changedY(1, 1), changedZ(1, 1); // 1 = unchanged
		for (size_t i = 0; i < m_meshVertices.size(); i++) {
			float3 p = m_meshVertices[i].position;
			if (p.x > m_MinMaxXPosition.y || changedX.y)
				m_MinMaxXPosition.y = p.x, changedX.y = 0;
			if (p.x < m_MinMaxXPosition.x || changedX.x)
				m_MinMaxXPosition.x = p.x, changedX.x = 0;

			if (p.y > m_MinMaxYPosition.y || changedY.y)
				m_MinMaxYPosition.y = p.y, changedY.y = 0;
			if (p.y < m_MinMaxYPosition.x || changedY.x)
				m_MinMaxYPosition.x = p.y, changedY.x = 0;

			if (p.z > m_MinMaxZPosition.y || changedZ.y)
				m_MinMaxZPosition.y = p.z, changedZ.y = 0;
			if (p.z < m_MinMaxZPosition.x || changedZ.x)
				m_MinMaxZPosition.x = p.z, changedZ.x = 0;
		}
		m_minmaxChanged = true;
		return true;
	}
	else
		return false;
}
void Mesh::updateBoundingBoxBuffer() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	if (m_boxVertices.size() > 0) {
		if (m_minmaxChanged) {
			m_minmaxChanged = false;
			float3 bbPos = getBoundingBoxPos();
			float3 bbScale = getBoundingBoxSize();
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
		ErrorLogger::logError(check, "Failed creating vertex buffer in Mesh class!\n");

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
			ErrorLogger::logError(check, "Failed creating boundingbox buffer in Mesh class!\n");
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
			ErrorLogger::logError(res, "Failed creating color buffer in Mesh class!\n");
	}
}
const std::vector<Vertex>& Mesh::getVertexPoints() const { return m_meshVertices; }
const Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::getVertexBuffer() const { return m_vertexBuffer; }
std::string Mesh::getName() const { return m_loadedMeshName; }
void Mesh::draw() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	if (m_materials.size() > 0)
		m_shaderObject.bindShadersAndLayout();
	else {
		draw_noMaterial();
		return;
	}

	bindMesh();

	for (size_t i = 0; i < m_parts.size(); i++) {
		for (size_t j = 0; j < m_parts[i].materialUsage.size(); j++) {
			int materialIndex = m_parts[i].materialUsage[j].materialIndex;
			// int materialIndex = findMaterial(parts[i].materialUsage[j].name);
			if (materialIndex != -1) {
				m_materials[materialIndex].bind(MATERIAL_BUFFER_SLOT);
				int count = m_parts[i].materialUsage[j].count;
				int index = m_parts[i].materialUsage[j].index;
				deviceContext->Draw(count, index);
			}
		}
	}
}
void Mesh::draw_noMaterial(float3 color) {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	m_shaderObject_onlyMesh.bindShadersAndLayout();

	bindMesh();

	// update color buffer
	float4 data = float4(color.x, color.y, color.z, 1.0);
	deviceContext->UpdateSubresource(m_colorBuffer.Get(), 0, 0, &data, 0, 0);
	deviceContext->PSSetConstantBuffers(COLOR_BUFFER_SLOT, 1, m_colorBuffer.GetAddressOf());

	deviceContext->Draw((UINT)m_meshVertices.size(), (UINT)0);
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
	deviceContext->Draw(36, 0);
}
void Mesh::draw_forShadowMap() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	bindMesh();

	deviceContext->Draw((UINT)m_meshVertices.size(), (UINT)0);
}
void Mesh::draw_withoutBinding() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	if (!(m_materials.size() > 0)) {
		draw_noMaterial();
		return;
	}

	for (size_t i = 0; i < m_parts.size(); i++) {
		for (size_t j = 0; j < m_parts[i].materialUsage.size(); j++) {
			int materialIndex = m_parts[i].materialUsage[j].materialIndex;
			// int materialIndex = findMaterial(parts[i].materialUsage[j].name);
			if (materialIndex != -1) {
				m_materials[materialIndex].bind(MATERIAL_BUFFER_SLOT);
				int count = m_parts[i].materialUsage[j].count;
				int index = m_parts[i].materialUsage[j].index;
				deviceContext->Draw(count, index);
			}
		}
	}
}
void Mesh::bindMesh() const {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}
float3 Mesh::getBoundingBoxPos() const {
	return float3((m_MinMaxXPosition.x + m_MinMaxXPosition.y) / 2,
		(m_MinMaxYPosition.x + m_MinMaxYPosition.y) / 2,
		(m_MinMaxZPosition.x + m_MinMaxZPosition.y) / 2);
}
float3 Mesh::getBoundingBoxSize() const {
	return float3((m_MinMaxXPosition.y - m_MinMaxXPosition.x),
			   (m_MinMaxYPosition.y - m_MinMaxYPosition.x),
			   (m_MinMaxZPosition.y - m_MinMaxZPosition.x)) /
		   2.f;
}
bool Mesh::load(std::string filename, bool combineParts) {
	if (m_handler.load(filename, m_meshVertices, m_parts, m_materials, combineParts)) {
		m_loadedMeshName = filename;
		findMinMaxValues();
		createBuffers();
		return true;
	}
	else
		return false;
}
float Mesh::castRayOnMesh(float3 rayPos, float3 rayDir) {
	// get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxSize();
	// check if close
	if (obbTest(rayDir, rayPos, bPos, bScale) != -1) {
		// find the exact point
		float closest = -1;
		size_t length = m_meshVertices.size() / 3;
		for (int i = 0; i < length; i++) {
			int index = i * 3;
			float3 v0 = m_meshVertices[index + 0].position;
			float3 v1 = m_meshVertices[index + 1].position;
			float3 v2 = m_meshVertices[index + 2].position;
			float t = triangleTest(rayDir, rayPos, v0, v1, v2);
			if ((t > 0 && t < closest) || closest < 0)
				closest = t;
		}
		return closest;
	}
	return -1;
}

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
