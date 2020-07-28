#include "Material.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include <WICTextureLoader.h>

Microsoft::WRL::ComPtr<ID3D11Buffer> Material::m_materialBuffer;

bool Material::createBuffers() {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	bool state = true;
	// material buffer
	if (m_materialBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(MaterialBuffer);

		HRESULT hrM = device->CreateBuffer(&desc, nullptr, m_materialBuffer.GetAddressOf());
		if (FAILED(hrM)) {
			ErrorLogger::messageBox(hrM, "Failed creating material buffer\n");
			state = false;
		}
	}

	return state;
}

bool Material::createResourceBuffer(int textureIndex, string textureName) {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	// create resource buffer
	m_maps[textureIndex].Reset();
	if (textureName != "") {
		string path = m_texturePath + textureName;
		wstring wstr = s2ws(path);
		LPCWCHAR str = wstr.c_str();
		HRESULT hrA = DirectX::CreateWICTextureFromFile(
			device, deviceContext, str, nullptr, m_maps[textureIndex].GetAddressOf());
		if (FAILED(hrA)) {
			ErrorLogger::messageBox(hrA, "(Material) Failed creating texturebuffer from texture\n" + path);
			m_maps[textureIndex].Reset();
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}

void Material::updateMaterialBuffer() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_materialBuffer.Get(), 0, 0, &m_data, 0, 0);
}

std::wstring Material::s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::string Material::getMaterialName() const { return m_materialName; }

VertexMaterialBuffer Material::convertToVertexBuffer() const {
	return VertexMaterialBuffer(float3(m_data.ambient3.x, m_data.ambient3.y, m_data.ambient3.z),
		float3(m_data.diffuse3_strength.x, m_data.diffuse3_strength.y, m_data.diffuse3_strength.z),
		float3(m_data.specular3_shininess.x, m_data.specular3_shininess.y,
			m_data.specular3_shininess.z),
		m_data.specular3_shininess.z);
}

void Material::setAmbientMap(std::string map_Ka) {
	m_ambientMap = map_Ka;
	m_data.mapUsages.x = 1;
	m_materialBufferChanged = true;

	createResourceBuffer(0, m_ambientMap);
}

void Material::setDiffuseMap(std::string map_Kd) {
	m_diffuseMap = map_Kd;
	m_data.mapUsages.y = 1;
	m_materialBufferChanged = true;

	createResourceBuffer(1, m_diffuseMap);
}

void Material::setSpecularMap(std::string map_Ks) {
	m_specularMap = map_Ks;
	m_data.mapUsages.z = 1;
	m_materialBufferChanged = true;

	createResourceBuffer(2, m_specularMap);
}

void Material::setAmbient(float3 a) {
	m_data.ambient3 = float4(a.x, a.y, a.z, m_data.ambient3.w);
	m_materialBufferChanged = true;
}

void Material::setDiffuse(float3 d) {
	m_data.diffuse3_strength = float4(d.x, d.y, d.z, m_data.diffuse3_strength.w);
	m_materialBufferChanged = true;
}

void Material::setSpecular(float3 s) {
	m_data.specular3_shininess = float4(s.x, s.y, s.z, m_data.diffuse3_strength.w);
	m_materialBufferChanged = true;
}

void Material::setSpecularHighlight(float Ns) {
	m_data.specular3_shininess.w = Ns;
	m_materialBufferChanged = true;
}

void Material::setDiffuseStrength(float Ni) {
	m_data.diffuse3_strength.w = Ni;
	m_materialBufferChanged = true;
}

void Material::bind(int materialBufferIndex, int resourceBufferIndex) {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	// bind material buffer
	deviceContext->PSSetConstantBuffers(materialBufferIndex, 1, m_materialBuffer.GetAddressOf());
	// update buffer
	deviceContext->UpdateSubresource(m_materialBuffer.Get(), 0, 0, &m_data, 0, 0);
	// bind texture maps
	for (int i = 0; i < 3; i++) {
		if (m_maps[i].Get() != nullptr)
			deviceContext->PSSetShaderResources(
				resourceBufferIndex + i, 1, m_maps[i].GetAddressOf());
	}
}

Material::Material(std::string _materialName, float3 _diffuse, float3 _ambient, float3 _specular,
	float _shininess, float _alpha) {

	m_data = MaterialBuffer(_diffuse, _ambient, _specular, _shininess, _alpha);
	m_materialName = _materialName;

	createBuffers();
}

Material::~Material() {}
