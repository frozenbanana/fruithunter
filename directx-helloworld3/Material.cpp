#include "Material.h"

bool Material::createBuffers() {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	bool state = true;
	//material buffer
	if (m_materialBuffer == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(MaterialBuffer);

		HRESULT state = device->CreateBuffer(&desc, nullptr, &m_materialBuffer);
		if (FAILED(state)) {
			ErrorLogger::messageBox(state,"Failed creating material buffer\n");
			state = false;
		}
	} 
	// textureBuffer
	m_maps = new ID3D11ShaderResourceView*[3];
	if (m_ambientMap != "") {
		string path = m_texturePath + m_ambientMap;
		wstring wstr = s2ws(path);
		LPCWCHAR str = wstr.c_str();
		HRESULT hr_a = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, &m_maps[0]);
		if (FAILED(hr_a)) {
			ErrorLogger::messageBox(hr_a, "Failed creating material buffer\n");
			state = false;
		}
	}
	else m_maps[0] = nullptr;
	if (m_diffuseMap != "") {
		string path = m_texturePath + m_diffuseMap;
		wstring wstr = s2ws(path);
		LPCWCHAR str = wstr.c_str();
		HRESULT hr_d = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, &m_maps[1]);
		if (FAILED(hr_d)) {
			ErrorLogger::messageBox(hr_d, "Failed creating material buffer\n");
			state = false;
		}
	}
	else m_maps[1] = nullptr;
	if (m_specularMap != "") {
		string path = m_texturePath + m_specularMap;
		wstring wstr = s2ws(path);
		LPCWCHAR str = wstr.c_str();
		HRESULT hr_s = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, &m_maps[2]);
		if (FAILED(hr_s)) {
			ErrorLogger::messageBox(hr_s, "Failed creating material buffer\n");
			state = false;
		}
	}
	else
		m_maps[2] = nullptr;
	
	return state;

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

void Material::setAmbientMap(std::string map_Ka) {
	m_ambientMap = map_Ka;
	m_data.mapUsages.x = 1;
}

void Material::setDiffuseMap(std::string map_Kd) {
	m_diffuseMap = map_Kd;
	m_data.mapUsages.y = 1;
}

void Material::setSpecularMap(std::string map_Ks) {
	m_diffuseMap = map_Ks;
	m_data.mapUsages.z = 1;
}

void Material::setAmbient(float3 a) { m_data.ambient3 = float4(a.x, a.y, a.z, m_data.ambient3.w); }

void Material::setDiffuse(float3 d) {
	m_data.diffuse3_strength = float4(d.x, d.y, d.z, m_data.diffuse3_strength.w); }

void Material::setSpecular(float3 s) {
	m_data.specular3_shininess = float4(s.x, s.y, s.z, m_data.diffuse3_strength.w); }

void Material::setSpecularHighlight(float Ns) { m_data.specular3_shininess.w = Ns; }

void Material::setDiffuseStrength(float Ni) { m_data.diffuse3_strength.w = Ni; }

Material::Material(
	std::string _materialName, float3 _diffuse, float3 _ambient, float3 _specular, float _shininess, float _alpha) {
	m_data = MaterialBuffer(_diffuse, _ambient, _specular, _shininess, _alpha);
	m_materialName = _materialName;
}
