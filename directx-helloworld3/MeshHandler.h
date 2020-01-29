#pragma once
#include "Material.h"

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
		std::string name;//material name
		int materialIndex = -1;//index of material arrays to use
		int index;//start index to triangles
		int count;//count of triangles using this material
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
	int count;//count of triangles on this part
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
	bool loadMTL(std::string fileName,std::vector<Material>& materials);
	/*loads an array of vertices*/
	bool loadRaw(std::string filename, std::vector<Vertex>& mesh);
	/*loads the object decription. also retreives material filename*/
	bool loadRawDesc(std::string filename,std::vector<Part>& parts);

	void saveToRaw(std::vector<Vertex>& mesh) const;
	void saveRawDesc(std::vector<Part>& parts) const;
	void flatShadeMesh(std::vector<Vertex>& mesh);

	/*
	 *	Merges all parts to one part. The mesh will be reconstructed and there will be only one part to use with correctly used materials.
	*/
	void combineParts(std::vector<Vertex>& mesh, std::vector<Part>& parts)const;
	/*
	 *	Fills the material index to use for the materialArray. This function is needed if materials are used as constant buffers
	*/
	void connectPartsToMaterialsInCorrectOrder(std::vector<Part>& parts, const std::vector<Material>& materials);
	/*
	 *	creates vertex buffer data needed for drawing with materials
	*/
	std::vector<VertexMaterialBuffer> createMaterialBufferData(const std::vector<Part>& parts, const std::vector<Material>& materials);

	void reset();
public:
	/*
	 *	Used if materials are used as constant buffers
	*/
	bool load(std::string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts, std::vector<Material>& materials,bool excludeParts = true);
	/*
	 *	Used if materials are used as vertex buffers
	*/
	bool load(string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts, std::vector<VertexMaterialBuffer>& vertex_materials, bool excludeParts = false);
	/*
	 *	Used if materials are not desired
	*/
	bool load(std::string filename, std::vector<Vertex>& mesh);

	MeshHandler();
	~MeshHandler();
};

