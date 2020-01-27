#pragma once
#include <fstream>
#include <WICTextureLoader.h>
#include <SimpleMath.h>
#include <vector>

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

struct Vertex {
	float3 position;
	float2 uv;
	float3 normal;
	Vertex(float3 _position = float3(0, 0, 0), float2 _uv = float2(0, 0), float3 _normal = float3(0, 0, 0)) {
		position = _position;
		uv = _uv;
		normal = _normal;
	}
};
struct Vertex_Material {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	UINT16 specularPower;
	Vertex_Material(float3 ambient = float3(1, 1, 1), float3 diffuse = float3(0.5, 0.5, 0.5),
		float3 specular = float3(1, 1, 1), UINT16 specularPower = 1) {
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->specularPower = specularPower;
	}
};

struct Material {
	float4 ambient3;
	float4 diffuse3_strength;//xyz diffuse, w strength for some strange reason
	float4 specular3_shininess;//xyz specular, w shininess
	float4 mapUsages = float4(0, 0, 0, 0);
	Material(float3 _diffuse = float3(1, 1, 1), float3 _ambient = float3(0.2, 0.2, 0.2), float3 _specular = float3(0, 0, 0), float _shininess = 1, float _alpha = 1) {
		diffuse3_strength = float4(_diffuse.x, _diffuse.y, _diffuse.z, 1);
		ambient3 = float4(_ambient.x, _ambient.y, _ambient.z, 1);
		specular3_shininess = float4(_specular.x, _specular.y, _specular.z, _shininess);
	}
};

struct MaterialPart {
	std::string materialName;
	std::string ambientMap = "";
	std::string diffuseMap = "";
	std::string specularMap = "";
	Material material;
	void setAmbientMap(std::string map_Ka) {
		ambientMap = map_Ka;
		material.mapUsages.x = 1;
	}
	void setDiffuseMap(std::string map_Kd) {
		diffuseMap = map_Kd;
		material.mapUsages.y = 1;
	}
	void setSpecularMap(std::string map_Ks) {
		diffuseMap = map_Ks;
		material.mapUsages.z = 1;
	}
	//material funcs
	void setAmbient(float3 a) {
		material.ambient3 = float4(a.x, a.y, a.z, material.ambient3.w);
	}
	void setDiffuse(float3 d) {
		material.diffuse3_strength = float4(d.x, d.y, d.z, material.diffuse3_strength.w);
	}
	void setSpecular(float3 s) {
		material.specular3_shininess = float4(s.x, s.y, s.z, material.diffuse3_strength.w);
	}
	void setSpecularHighlight(float Ns) {
		material.specular3_shininess.w = Ns;
	}
	void setDiffuseStrength(float Ni) {
		material.diffuse3_strength.w = Ni;
	}
	//constructor
	MaterialPart(std::string _materialName = "noName", float3 _diffuse = float3(1, 1, 1), float3 _ambient = float3(0.2, 0.2, 0.2), float3 _specular = float3(0, 0, 0), float _shininess = 1, float _alpha = 1) {
		material = Material(_diffuse, _ambient, _specular, _shininess, _alpha);
		materialName = _materialName;
	}
};
struct VertexRef {
	int position, uv, normal;
	VertexRef(int _position = -1, int _uv = -1, int _normal = -1) {
		position = _position;
		uv = _uv;
		normal = _normal;
	}
};
struct Part {
	struct MaterialUsage {
		std::string name;
		int index;
		int count;
		MaterialUsage(std::string name = "", int index = 0) {
			this->name = name;
			this->index = index;
			count = 0;
		}
		void countUp(int n = 1) {
			count += n;
		}
	};
	std::string name;//name of part
	int index;//start index to triangles
	int count;//count of triangles using
	std::vector<MaterialUsage> materialUsage;
	Part(std::string name = "",int index = 0) {
		this->name = name;
		this->index = index;
		this->count = 0;
	}
	void newMaterial(std::string name) {
		materialUsage.push_back(MaterialUsage(name,index+count));
	}
	void countUp(int n) {
		count += n;
		materialUsage.back().countUp(n);
	}
};

class MeshHandler
{
private:
	std::string loadedObjName = "";

	const std::string mtlPath = "Meshes/MTL/";
	const std::string objPath = "Meshes/OBJ/";
	const std::string rawPath = "Meshes/RAW/";

	std::vector<float3> vertices_position;
	std::vector<float2> vertices_uv;
	std::vector<float3> vertices_normal;

	std::vector<VertexRef> triangleRefs;

	std::string materialFileName = "";

	Vertex createVertexFromRef(const VertexRef& ref) const;
	std::vector<VertexRef> triangulate(std::vector<VertexRef> face);

	/*creates an array of vertices from position, uv, normal and triangleRefs arrays*/
	std::vector<Vertex> createMesh() const;
	bool preCheckOBJ(std::string filename, int& positions,int& uvs, int& normals, int& triangles);
	/*loads arrays with position, uvs, normals. creates parts. fetches material filename*/
	bool loadOBJ(std::string fileName, std::vector<Part>& parts);
	/*loads materials*/
	bool loadMTL(std::string fileName,std::vector<MaterialPart>& materials);
	/*loads an array of vertices*/
	bool loadRaw(std::string filename, std::vector<Vertex>& mesh);
	/*loads the object decription. also retreives material filename*/
	bool loadRawDesc(std::string filename,std::vector<Part>& parts);

	void saveToRaw(std::vector<Vertex>& mesh) const;
	void saveRawDesc(std::vector<Part>& parts) const;
	void flatShadeMesh(std::vector<Vertex>& mesh);

	void combineParts(std::vector<Vertex>& mesh, std::vector<Part>& parts)const;
	std::vector<Vertex_Material> createMaterialBufferData(const std::vector<Part>& parts, const std::vector<MaterialPart>& materials);

	void reset();
public:

	bool load(std::string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts, std::vector<MaterialPart>& materials,bool excludeParts = false);
	bool load(std::string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts, std::vector<Vertex_Material>& vertex_materials,bool excludeParts = false);
	bool load(std::string filename, std::vector<Vertex>& mesh);

	MeshHandler();
	~MeshHandler();
};

