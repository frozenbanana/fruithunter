#include "Mesh.h"

int Mesh::meshCount = 0;
std::vector<Vertex> Mesh::box;
ShaderSet Mesh::shader_object;
ShaderSet Mesh::shader_object_onlyMesh;
ID3D11Buffer* Mesh::materialBuffer = nullptr;
ID3D11Buffer* Mesh::vertexBuffer_BoundingBox = nullptr;

float Mesh::triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2)
{
	float3 normal = ((tri1 - tri0).Cross(tri2 - tri0)); normal.Normalize();
	float3 toTri = rayOrigin - tri0; toTri.Normalize();
	float proj = toTri.Dot(normal);
	if (proj < 0)return -1;
	//mat3 m = mat3(-rayDir, tri.vtx1.xyz - tri.vtx0.xyz, tri.vtx2.xyz - tri.vtx0.xyz);
	float4x4 m(-rayDir, tri1 - tri0, tri2 - tri0);
	float3 op0 = rayOrigin - tri0;
	float3 tuv = XMVector3Transform(op0, m.Invert());
	float4 tuvw = float4(tuv.x, tuv.y, tuv.z, 0);
	tuvw.w = 1 - tuvw.y - tuvw.z;
	if (tuvw.y >= 0 && tuvw.y <= 1 && tuvw.z >= 0 && tuvw.z <= 1 && tuvw.w >= 0 && tuvw.w <= 1)
		return tuvw.x;
	else return -1;
}

float Mesh::obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale)
{
	//SLABS CALULATIONS(my own)
	float4 data[3] = { float4(1,0,0,boxScale.x),float4(0,1,0,boxScale.y),float4(0,0,1,boxScale.z) };//{o.u_hu,o.v_hv,o.w_hw};
	float Tmin = -99999999, Tmax = 9999999999;
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
				t1 = t2; t2 = temp;
			}
			if (t1 > Tmin) {
				Tmin = t1;
			}
			if (t2 < Tmax) Tmax = t2;
		}
		else return -1;
	}
	if (Tmin < Tmax) return Tmin;
	else return -1;
}
bool Mesh::findMinMaxValues()
{
	if (mesh.size() > 0) {
		DirectX::XMINT2 changedX(1, 1), changedY(1, 1), changedZ(1, 1);//1 = unchanged
		for (int i = 0; i < mesh.size(); i++)
		{
			float3 p = mesh[i].position;
			if (p.x > MinMaxXPosition.y || changedX.y)MinMaxXPosition.y = p.x, changedX.y = 0;
			if (p.x < MinMaxXPosition.x || changedX.x)MinMaxXPosition.x = p.x, changedX.x = 0;

			if (p.y > MinMaxYPosition.y || changedY.y)MinMaxYPosition.y = p.y, changedY.y = 0;
			if (p.y < MinMaxYPosition.x || changedY.x)MinMaxYPosition.x = p.y, changedY.x = 0;

			if (p.z > MinMaxZPosition.y || changedZ.y)MinMaxZPosition.y = p.z, changedZ.y = 0;
			if (p.z < MinMaxZPosition.x || changedZ.x)MinMaxZPosition.x = p.z, changedZ.x = 0;
		}
		minmaxChanged = true;
		return true;
	}
	else return false;
}
void Mesh::updateBoundingBoxBuffer()
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();
	if (box.size() > 0) {
		if (minmaxChanged) {
			minmaxChanged = false;
			float3 bbPos = getBoundingBoxPos();
			float3 bbScale = getBoundingBoxSize();
			std::vector<Vertex> fullBox; fullBox.resize(36);
			for (int i = 0; i < 36; i++)
			{
				Vertex v = box[i];
				v.position = v.position*bbScale + bbPos;
				fullBox.push_back(v);
			}
			gDeviceContext->UpdateSubresource(vertexBuffer_BoundingBox, 0, 0, fullBox.data(), 0, 0);
		}
	}
}
void Mesh::loadBoundingBox()
{
	//vertices
	box.resize(36);
	float2 pre[4] = { float2(-1,1),float2(1,1),float2(-1,-1),float2(1,-1) };
	for (int f = 0; f < 3; f++)//per slab .3
	{
		for (int j = 0; j < 2; j++)//parallel planes .2
		{
			int m = j * 2 - 1;
			Vertex v[4];
			for (int i = 0; i < 4; i++)//per point
			{
				float2 a = pre[i];
				float3 p, n;
				if (f == 0)
					p = float3(a.y, a.x, 0) + float3(0, 0, m), n = float3(0, 0, m);
				else if (f == 1)
					p = float3(a.x, 0, a.y) + float3(0, m, 0), n = float3(0, m, 0);
				else if (f == 2)
					p = float3(0, a.y, a.x) + float3(m, 0, 0), n = float3(m, 0, 0);
				float2 uv(a.x, -a.y);
				v[i] = Vertex(p, uv, n);
			}
			if (m == 1)
				box.push_back(v[0]), box.push_back(v[1]), box.push_back(v[3]),
				box.push_back(v[0]), box.push_back(v[3]), box.push_back(v[2]);
			else
				box.push_back(v[1]), box.push_back(v[0]), box.push_back(v[3]),
				box.push_back(v[2]), box.push_back(v[3]), box.push_back(v[0]);
		}
	}
}
void Mesh::createBuffers(bool instancing)
{
	ID3D11DeviceContext* gDevice = Renderer::getDevice();
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	HRESULT check;

	freeBuffers();
	//vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = mesh.size()*sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = mesh.data();

	check = gDevice->CreateBuffer(&bufferDesc, &data, &vertexBuffer);

	//vertex material buffer
	D3D11_BUFFER_DESC bufferMatDesc;
	memset(&bufferMatDesc, 0, sizeof(bufferMatDesc));

	bufferMatDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferMatDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferMatDesc.ByteWidth = sizeof(Vertex_Material)*mesh_materials.size();

	D3D11_SUBRESOURCE_DATA matData;
	matData.pSysMem = mesh_materials.data();

	check = gDevice->CreateBuffer(&bufferMatDesc, &matData, &vertexMaterialBuffer);

	//material buffer
	if (materialBuffer == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Material);

		check = gDevice->CreateBuffer(&desc, nullptr, &materialBuffer);
	}
	//boundingbox buffer
	if (vertexBuffer_BoundingBox == nullptr) {
		//buffer
		D3D11_BUFFER_DESC buffDesc;
		memset(&buffDesc, 0, sizeof(buffDesc));

		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		buffDesc.ByteWidth = sizeof(Vertex)*36;

		check = gDevice->CreateBuffer(&buffDesc, nullptr, &vertexBuffer_BoundingBox);
	}
	//textureBuffer
	if (materials.size() > 0) {
		maps = new ID3D11ShaderResourceView**[materials.size()];
		for (size_t i = 0; i < materials.size(); i++)
		{
			maps[i] = new ID3D11ShaderResourceView*[3];
			if (materials[i].ambientMap != "") {
				std::string path = "Meshes/" + materials[i].ambientMap;
				std::wstring wstr = s2ws(path);
				LPCWCHAR str = wstr.c_str();
				HRESULT hr_a = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][0]);
			}
			else maps[i][0] = nullptr;
			if (materials[i].diffuseMap != "") {
				std::string path = "Meshes/" + materials[i].diffuseMap;
				std::wstring wstr = s2ws(path);
				LPCWCHAR str = wstr.c_str();
				HRESULT hr_d = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][1]);
			}
			else maps[i][1] = nullptr;
			if (materials[i].specularMap != "") {
				std::string path = "Meshes/" + materials[i].specularMap;
				std::wstring wstr = s2ws(path);
				LPCWCHAR str = wstr.c_str();
				HRESULT hr_s = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][2]);
			}
			else maps[i][2] = nullptr;
		}
	}
}
void Mesh::freeBuffers()
{
	if (vertexBuffer != nullptr) {
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}
	if (maps != nullptr) {
		for (int i = 0; i < materials.size(); i++)
		{
			if (maps[i] != nullptr) {
				if (maps[i][0] != nullptr)maps[i][0]->Release();
				if (maps[i][1] != nullptr)maps[i][1]->Release();
				if (maps[i][2] != nullptr)maps[i][2]->Release();
				delete[] maps[i];
			}
		}
		delete[] maps;
		maps = nullptr;
	}
	if (meshCount == 0) {
		if (materialBuffer != nullptr)materialBuffer->Release();
		if (vertexBuffer_BoundingBox != nullptr)vertexBuffer_BoundingBox->Release();
	}
}
int Mesh::findMaterial(std::string name) const
{
	for (int i = 0; i < materials.size(); i++)
	{
		if (materials[i].materialName == name)
			return i;
	}
	return -1;
}
const std::vector<Vertex>& Mesh::getVertexPoints() const
{
	return mesh;
}
std::string Mesh::getName() const
{
	return loadedMeshName;
}
void Mesh::draw()
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	//if(materials.length() > 0)shader_object.bindShadersAndLayout();
	//else {
	//	draw_noMaterial();
	//	return;
	//}
	//UINT strides = sizeof(Vertex);
	//UINT offset = 0;
	//gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);
	//gDeviceContext->PSSetConstantBuffers(2, 1, &materialBuffer);

	//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	////gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//for (int i = 0; i < parts.length(); i++)
	//{
	//	for (int j = 0; j < parts[i].materialUsage.length(); j++)
	//	{
	//		int materialIndex = findMaterial(parts[i].materialUsage[j].name);
	//		if (materialIndex != -1) {
	//			gDeviceContext->UpdateSubresource(materialBuffer, 0, 0, &materials[materialIndex].material, 0, 0);
	//			gDeviceContext->PSSetShaderResources(0, 3, maps[materialIndex]);
	//			gDeviceContext->Draw(parts[i].materialUsage[j].count, parts[i].materialUsage[j].index);
	//		}
	//	}
	//}


	if (mesh_materials.size() > 0)shader_object.bindShadersAndLayout();
	else {
		draw_noMaterial();
		return;
	}
	ID3D11Buffer* buffs[] = { vertexBuffer,vertexMaterialBuffer };
	UINT strides[] = { sizeof(Vertex) ,sizeof(Vertex_Material) };
	UINT offset[] = { 0,0 };
	gDeviceContext->IASetVertexBuffers(0, 2, buffs, strides, offset);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gDeviceContext->Draw(mesh.size(),0);
}
void Mesh::draw_noMaterial(float3 color)
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	shader_object_onlyMesh.bindShadersAndLayout();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	updateColorBuffer(color);
	gDeviceContext->Draw(mesh.size(), 0);
}
void Mesh::draw_BoundingBox()
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	updateBoundingBoxBuffer();
	shader_object_onlyMesh.bindShadersAndLayout();
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer_BoundingBox, &strides, &offset);
	gDeviceContext->Draw(36,0);
}
void Mesh::draw_forShadowMap()
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < parts.size(); i++)
	{
		gDeviceContext->Draw(parts[i].count, parts[i].index);
	}
}
void Mesh::bindMesh() const
{
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
float3 Mesh::getBoundingBoxPos() const
{
	return float3((MinMaxXPosition.x + MinMaxXPosition.y) / 2, (MinMaxYPosition.x + MinMaxYPosition.y) / 2, (MinMaxZPosition.x + MinMaxZPosition.y) / 2);
}
float3 Mesh::getBoundingBoxSize() const
{
	return float3((MinMaxXPosition.y - MinMaxXPosition.x), (MinMaxYPosition.y - MinMaxYPosition.x), (MinMaxZPosition.y - MinMaxZPosition.x));
}
bool Mesh::load(std::string filename, bool combineParts, bool instancing)
{
	/*if (handler.load(filename, mesh, parts, materials,combineParts)) {
		loadedMeshName = filename;
		findMinMaxValues(); 
		createBuffers();
		return true;
	}
	else return false;*/
	if (handler.load(filename, mesh, parts, mesh_materials, combineParts)) {
		loadedMeshName = filename;
		findMinMaxValues();
		createBuffers();
		return true;
	}
	else return false;
}
float Mesh::castRayOnMesh(float3 rayPos, float3 rayDir)
{
	//get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxSize();
	//check if close
	if (obbTest(rayDir, rayPos, bPos, bScale) != -1) {
		//find the exact point
		float closest = -1;
		int length = mesh.size() / 3;
		for (int i = 0; i < length; i++)
		{
			int index = i * 3;
			float3 v0 = mesh[index + 0].position;
			float3 v1 = mesh[index + 1].position;
			float3 v2 = mesh[index + 2].position;
			float t = triangleTest(rayDir, rayPos, v0, v1, v2);
			if ((t > 0 && t < closest) || closest < 0)closest = t;
		}
		return closest;
	}
	return -1;
}

Mesh::Mesh(std::string OBJFile)
{
	D3D11_INPUT_ELEMENT_DESC inputLayout_onlyMesh[] = {
		{
			"Position",		// "semantic" name in shader
			0,				// "semantic" index (not used)
			DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
			0,							 // input slot
			0,							 // offset of first element
			D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
			0							 // used for INSTANCING (ignore)
		},
		{
			"TexCoordinate",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			12,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Normal",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			20,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	D3D11_INPUT_ELEMENT_DESC inputLayout_withMaterial[] = {
		{
			"Position",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Texcoord",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			12,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Normal",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			20,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Ambient",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			1,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Diffuse",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			1,
			12,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"Specular",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			1,
			24,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"SpecularPower",
			0,
			DXGI_FORMAT_R32_UINT,
			1,
			36,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	if (!shader_object_onlyMesh.isLoaded())shader_object_onlyMesh.createShaders(L"Effects/Vertex.hlsl", nullptr, L"Effects/Fragment_onlyMesh.hlsl", inputLayout_onlyMesh,3);
	if (!shader_object.isLoaded())shader_object.createShaders(L"Effects/Vertex_MeshWithMaterial.hlsl", nullptr, L"Effects/Fragment_MeshWithMaterial.hlsl", inputLayout_withMaterial,7);
	meshCount++;
	if (box.size() == 0) {
		loadBoundingBox();
	}
	if (OBJFile != "")
		load(OBJFile);
}
Mesh & Mesh::operator=(const Mesh & other)
{
	minmaxChanged = other.minmaxChanged;
	MinMaxXPosition = other.MinMaxXPosition;
	MinMaxYPosition = other.MinMaxYPosition;
	MinMaxZPosition = other.MinMaxZPosition;
	parts = other.parts;
	mesh = other.mesh;
	materials = other.materials;

	findMinMaxValues();
	createBuffers();

	return *this;
}
Mesh::~Mesh()
{
	meshCount--;
	freeBuffers();
}

