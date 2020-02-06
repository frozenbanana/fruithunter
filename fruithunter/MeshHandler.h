#pragma once
#include "Material.h"

struct Vertex {
	float3 position;
	float2 uv;
	float3 normal;
	Vertex(float3 _position = float3(0.f, 0.f, 0.f), float2 _uv = float2(0.f, 0.f),
		float3 _normal = float3(0.f, 0.f, 0.f)) {
		position = _position;
		uv = _uv;
		normal = _normal;
	}
};

struct Part {
	struct MaterialUsage {
		std::string name;		// material name
		int materialIndex = -1; // index of material arrays to use
		int index;				// start index to triangles
		int count;				// count of triangles using this material
		MaterialUsage(std::string name = "", int index = 0) {
			this->name = name;
			this->index = index;
			count = 0;
		}
		void countUp(int n = 1) { count += n; }
	};
	std::string name; // name of part
	int index;		  // start index to triangles
	int count;		  // count of triangles on this part
	std::vector<MaterialUsage> materialUsage;
	Part(std::string name = "", int index = 0) {
		this->name = name;
		this->index = index;
		this->count = 0;
	}
	void newMaterial(std::string name) {
		materialUsage.push_back(MaterialUsage(name, index + count));
	}
	void countUp(int n) {
		count += n;
		materialUsage.back().countUp(n);
	}
};

class MeshHandler {
private:
	struct VertexRef {
		int position, uv, normal;
		VertexRef(int _position = -1, int _uv = -1, int _normal = -1) {
			position = _position;
			uv = _uv;
			normal = _normal;
		}
	};
	std::string m_loadedObjName = "";

	const std::string m_mtlPath = "Meshes/MTL/";
	const std::string m_objPath = "Meshes/OBJ/";
	const std::string m_rawPath = "Meshes/RAW/";

	std::vector<float3> m_vertices_position;
	std::vector<float2> m_vertices_uv;
	std::vector<float3> m_vertices_normal;

	std::vector<MeshHandler::VertexRef> m_triangleRefs;

	std::string m_materialFileName = "";

	Vertex createVertexFromRef(const MeshHandler::VertexRef& ref) const;
	std::vector<MeshHandler::VertexRef> triangulate(std::vector<MeshHandler::VertexRef> face);

	/*creates an array of vertices from position, uv, normal and triangleRefs arrays*/
	std::vector<Vertex> createMesh() const;
	bool preCheckOBJ(std::string filename, int& positions, int& uvs, int& normals, int& triangles);
	/*loads arrays with position, uvs, normals. creates parts. fetches material filename*/
	bool loadOBJ(std::string fileName, std::vector<Part>& parts);
	/*loads materials*/
	bool loadMTL(std::string fileName, std::vector<Material>& materials);
	/*loads an array of vertices*/
	bool loadRaw(std::string filename, std::vector<Vertex>& mesh);
	/*loads the object decription. also retreives material filename*/
	bool loadRawDesc(std::string filename, std::vector<Part>& parts);

	/*
	 *	Saves vertices data in a .rw file, saved binary, increasing load time
	 */
	void saveToRaw(std::vector<Vertex>& mesh) const;
	/*
	 *	Saves Part description to a .rwd file
	 */
	void saveRawDesc(std::vector<Part>& parts) const;
	/*
	 *	Forces the loaded vertices to have normals pointing in their triangle direction
	 */
	void flatShadeMesh(std::vector<Vertex>& mesh);

	/*
	 *	Merges all parts to one part. The mesh will be reconstructed and there will be only one part
	 *left with correctly merged materials.
	 */
	void combineParts(std::vector<Vertex>& mesh, std::vector<Part>& parts) const;
	/*
	 *	Fills the material index to use for the materialArray. This function is needed if materials
	 *are used as constant buffers
	 */
	void connectPartsToMaterialsInCorrectOrder(
		std::vector<Part>& parts, const std::vector<Material>& materials);

	void reset();

public:
	/*
	 *	Used if materials are used as constant buffers
	 */
	bool load(std::string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts,
		std::vector<Material>& materials, bool excludeParts = true);
	/*
	 *	Used if materials are not desired
	 */
	bool load(std::string filename, std::vector<Vertex>& mesh);

	MeshHandler();
	~MeshHandler();
};
