#pragma once
#include "GlobalNamespaces.h"

struct VertexMaterialBuffer {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float specularPower;
	VertexMaterialBuffer(float3 ambient = float3(1.f, 1.f, 1.f),
		float3 diffuse = float3(0.5f, 0.5f, 0.5f), float3 specular = float3(1.f, 1.f, 1.f),
		float specularPower = 1.f) {
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->specularPower = specularPower;
	}
};

class Material {
private:
	const string m_materialPath = "Meshes/MTL/";
	const string m_texturePath = "Meshes/Textures/";
	// variables
	string m_materialName;
	string m_ambientMap = "";
	string m_diffuseMap = "";
	string m_specularMap = "";

	struct MaterialBuffer {
		float4 ambient3;
		float4 diffuse3_strength;	// xyz diffuse, w strength for some strange reason
		float4 specular3_shininess; // xyz specular, w shininess
		float4 mapUsages = float4(0.f, 0.f, 0.f, 0.f);
		MaterialBuffer(float3 _diffuse = float3(1.f, 1.f, 1.f),
			float3 _ambient = float3(0.2f, 0.2f, 0.2f), float3 _specular = float3(0.f, 0.f, 0.f),
			float _shininess = 1.f, float _alpha = 1.f) {
			diffuse3_strength = float4(_diffuse.x, _diffuse.y, _diffuse.z, 1.f);
			ambient3 = float4(_ambient.x, _ambient.y, _ambient.z, 1.f);
			specular3_shininess = float4(_specular.x, _specular.y, _specular.z, _shininess);
		}
	} m_data;
	bool m_materialBufferChanged = false;
	// buffers
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_maps[3];

	bool createBuffers();
	bool createResourceBuffer(int textureIndex, string textureName);
	void updateMaterialBuffer();

	std::wstring s2ws(const std::string& s);

public:
	std::string getMaterialName() const;
	VertexMaterialBuffer convertToVertexBuffer() const;

	void setAmbientMap(std::string map_Ka);
	void setDiffuseMap(std::string map_Kd);
	void setSpecularMap(std::string map_Ks);
	void setAmbient(float3 a);
	void setDiffuse(float3 d);
	void setSpecular(float3 s);
	void setSpecularHighlight(float Ns);
	void setDiffuseStrength(float Ni);


	void bind(int materialBufferIndex = 2, int resourceBufferIndex = 0);

	Material(std::string _materialName = "noName", float3 _diffuse = float3(1.f, 1.f, 1.f),
		float3 _ambient = float3(0.2f, 0.2f, 0.2f), float3 _specular = float3(0.f, 0.f, 0.f),
		float _shininess = 1.f, float _alpha = 1.f);

	~Material();
};
