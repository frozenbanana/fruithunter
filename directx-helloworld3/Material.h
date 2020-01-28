#pragma once
#include "Renderer.hpp"
#include "ErrorLogger.hpp"
#include <fstream>
#include <WICTextureLoader.h>
#include <SimpleMath.h>
#include <vector>
#include <string>

using namespace std;

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

class Material {
private:
	//variables
	string m_materialName;
	string m_ambientMap = "";
	string m_diffuseMap = "";
	string m_specularMap = "";

	struct MaterialBuffer {
		float4 ambient3;
		float4 diffuse3_strength;	// xyz diffuse, w strength for some strange reason
		float4 specular3_shininess; // xyz specular, w shininess
		float4 mapUsages = float4(0, 0, 0, 0);
		MaterialBuffer(float3 _diffuse = float3(1, 1, 1), float3 _ambient = float3(0.2, 0.2, 0.2),
			float3 _specular = float3(0, 0, 0), float _shininess = 1, float _alpha = 1) {
			diffuse3_strength = float4(_diffuse.x, _diffuse.y, _diffuse.z, 1);
			ambient3 = float4(_ambient.x, _ambient.y, _ambient.z, 1);
			specular3_shininess = float4(_specular.x, _specular.y, _specular.z, _shininess);
		}
	} m_data;
	//buffers
	ID3D11Buffer* m_materialBuffer = nullptr;
	ID3D11ShaderResourceView** m_maps = nullptr;

	bool createBuffers();

public:
	void setAmbientMap(std::string map_Ka);
	void setDiffuseMap(std::string map_Kd);
	void setSpecularMap(std::string map_Ks);
	// material funcs
	void setAmbient(float3 a);
	void setDiffuse(float3 d);
	void setSpecular(float3 s);
	void setSpecularHighlight(float Ns);
	void setDiffuseStrength(float Ni);
	// constructor
	Material(std::string _materialName = "noName", float3 _diffuse = float3(1, 1, 1),
		float3 _ambient = float3(0.2, 0.2, 0.2), float3 _specular = float3(0, 0, 0), float _shininess = 1,
		float _alpha = 1);
};
