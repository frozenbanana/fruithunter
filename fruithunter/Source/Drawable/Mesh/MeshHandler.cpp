#include "MeshHandler.h"
#include "ErrorLogger.h"

#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

Vertex MeshHandler::createVertexFromRef(const MeshHandler::VertexRef& ref) const {
	Vertex v;
	if (ref.position >= 0)
		v.position = m_vertices_position[ref.position];
	if (ref.uv >= 0)
		v.uv = m_vertices_uv[ref.uv];
	if (ref.normal >= 0)
		v.normal = m_vertices_normal[ref.normal];
	return v;
}

std::vector<MeshHandler::VertexRef> MeshHandler::triangulate(
	std::vector<MeshHandler::VertexRef> face) {
	std::vector<MeshHandler::VertexRef> ret;
	if (face.size() == 3) {
		ret.push_back(face[0]);
		ret.push_back(face[1]);
		ret.push_back(face[2]);
	}
	else if (face.size() == 4) {
		ret.push_back(face[0]);
		ret.push_back(face[1]);
		ret.push_back(face[2]);

		ret.push_back(face[2]);
		ret.push_back(face[3]);
		ret.push_back(face[0]);
	}
	return ret;
}

bool MeshHandler::loadOBJ(std::string fileName, std::vector<Part>& parts) {
	int posCount = 0, normalCount = 0, uvCount = 0, triangleCount = 0;
	if (preCheckOBJ(fileName, posCount, uvCount, normalCount, triangleCount)) {

		std::fstream objFile;
		objFile.open(PATH_OBJ + fileName + ".obj", std::ios::in);
		if (objFile.is_open()) {
			// reset
			m_vertices_position.reserve(posCount);
			m_vertices_uv.reserve(uvCount);
			m_vertices_normal.reserve(normalCount);
			m_triangleRefs.reserve(triangleCount);
			parts.clear();
			m_loadedObjName = fileName;

			Part* part = nullptr;
			std::string startWord = "";
			while (objFile.peek() != EOF) {
				objFile >> startWord;
				if (startWord[0] == '#') { // comment
					char c[100];
					objFile.getline(c, 100);
				}
				else if (startWord == "mtllib") { // mtl file name
					objFile >> m_materialFileName;
				}
				else if (startWord == "o") { // new part with part name
					std::string name = "";
					objFile >> name;
					parts.push_back(Part(name, (int)m_triangleRefs.size()));
					part = &parts.back();
				}
				else if (startWord == "usemtl") { // new faces repository for current meshPart
					std::string name = "";
					objFile >> name;
					part->newMaterial(name);
				}
				else if (startWord == "v") { // vertex position
					float3 pos;
					objFile >> pos.x >> pos.y >> pos.z;
					m_vertices_position.push_back(pos);
				}
				else if (startWord == "vn") { // normal
					float3 norm;
					objFile >> norm.x >> norm.y >> norm.z;
					m_vertices_normal.push_back(norm);
				}
				else if (startWord == "vt") { // uv coord
					float2 uv;
					objFile >> uv.x >> uv.y;
					uv.y = 1 - uv.y; // UV IS UPSIDE DOWN!!!!
					m_vertices_uv.push_back(uv);
				}
				else if (startWord == "f") { // face
					std::vector<MeshHandler::VertexRef> face;
					while (objFile.peek() != '\n' && objFile.peek() != EOF) {
						int pI = -1, uvI = -1, nI = -1;
						char t;
						// position index
						objFile >> pI;
						if (objFile.peek() == '/') {
							objFile >> t;
							if (objFile.peek() == '/') {
								objFile >> t >> nI;
							}
							else {
								objFile >> uvI;
								if (objFile.peek() == '/') {
									objFile >> t >> nI;
								}
							}
						}
						face.push_back(MeshHandler::VertexRef(pI - 1, uvI - 1, nI - 1));
					}
					std::vector<MeshHandler::VertexRef> temp = triangulate(face);
					part->countUp((int)temp.size());
					// merge arrays
					for (size_t i = 0; i < temp.size(); i++) {
						m_triangleRefs.push_back(temp[i]);
					}
				}
			}
			objFile.close();
			return true;
		}
		return false;
	}
	return false;
}

bool MeshHandler::load(std::string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts,
	std::vector<Material>& materials, bool excludeParts) {
	bool success = true;
	if (filename != "") {
		if (isValidRAW(filename) && loadRaw(filename, mesh) && loadRawDesc(filename, parts)) {
			// success
			if (excludeParts)
				combineParts(mesh, parts);
			if (loadMTL(m_materialFileName, materials)) {
				connectPartsToMaterialsInCorrectOrder(parts, materials);
			}
			else {
				// failed loading material
				ErrorLogger::logWarning("WARNING! MeshHandler failed at reading the file: " +
										m_materialFileName + " for mesh: " + filename);
				// success is still true! Mesh will be forced rendered without material
			}
		}
		else {
			// failure
			if (loadOBJ(filename, parts)) {
				mesh = createMesh();
				saveToRaw(mesh);
				saveRawDesc(parts);
				if (excludeParts)
					combineParts(mesh, parts);
				if (loadMTL(m_materialFileName, materials)) {
					connectPartsToMaterialsInCorrectOrder(parts, materials);
				}
				else {
					// failed loading material
					ErrorLogger::logWarning(
						"WARNING! MeshHandler failed at reading the file: " + m_materialFileName +
						" for mesh: " + filename); // failed loading material
					// success is still true! Mesh will be forced rendered without material
				}
			}
			else {
				if (loadOBJ(filename, parts)) {
					ErrorLogger::logWarning(
						"WARNING! MeshHandler failed at reading .obj file: " + filename);
				}
				else {
					success = false; // failed loading .obj and .rw file
				}
			}
		}
	}
	else
		success = false; // filename empty!

	reset();
	return success;
}

bool MeshHandler::load(std::string filename, std::vector<Vertex>& mesh) {
	bool success = true;
	if (filename != "") {
		std::vector<Part> parts;
		if (isValidRAW(filename) && loadRaw(filename, mesh)) {}
		else if (loadOBJ(filename, parts)) {
			mesh = createMesh();
		}
		else {
			ErrorLogger::logWarning(
				"WARNING! MeshHandler failed at reading .obj file: " + filename);
			success = false; // failed loading .obj and .rw file
		}
	}
	else
		success = false; // filename empty!
	return success;
}

std::vector<Vertex> MeshHandler::createMesh() const {
	std::vector<Vertex> arr;
	if (m_triangleRefs.size() > 0) {
		arr.reserve(m_triangleRefs.size());
		for (size_t i = 0; i < m_triangleRefs.size() / 3; i++) {
			int index = (int)i * 3;
			arr.push_back(createVertexFromRef(m_triangleRefs[index + 0]));
			arr.push_back(createVertexFromRef(m_triangleRefs[index + (short int)1]));
			arr.push_back(createVertexFromRef(m_triangleRefs[index + (short int)2]));
		}
	}
	return arr;
}

bool MeshHandler::preCheckOBJ(
	std::string filename, int& positions, int& uvs, int& normals, int& triangles) {
	std::fstream objFile;
	objFile.open(PATH_OBJ + filename + ".obj", std::ios::in);
	if (objFile.is_open()) {
		Part* part = nullptr;

		std::string startWord = "";
		while (objFile.peek() != EOF) {
			objFile >> startWord;
			if (startWord[0] == '#') { // comment
				char c[100];
				objFile.getline(c, 100);
			}
			else if (startWord == "v") { // vertex position
				positions++;
			}
			else if (startWord == "vn") { // normal
				normals++;
			}
			else if (startWord == "vt") { // uv coord
				uvs++;
			}
			else if (startWord == "f") { // face
				int facePoints = 0;
				while (objFile.peek() != '\n' && objFile.peek() != EOF) {
					int pI = -1, uvI = -1, nI = -1;
					char t;
					// position index
					objFile >> pI;
					if (objFile.peek() == '/') {
						objFile >> t;
						if (objFile.peek() == '/') {
							objFile >> t >> nI;
						}
						else {
							objFile >> uvI;
							if (objFile.peek() == '/') {
								objFile >> t >> nI;
							}
						}
					}
					facePoints++;
				}
				if (facePoints == 3)
					triangles += 3;
				else if (facePoints == 4)
					triangles += 6;
			}
			else {
				char c[100];
				objFile.getline(c, 100);
			}
		}
		objFile.close();
		return true;
	}
	return false;
}

bool MeshHandler::loadMTL(std::string filename, std::vector<Material>& materials) {
	std::fstream mtlFile;
	mtlFile.open(PATH_MTL + filename);
	if (mtlFile.is_open()) {
		// temp variables
		std::string text = "";
		float x = 0, y = 0, z = 0;
		float floatValue = 0;
		// read file
		std::string startWord = "";
		while (mtlFile.peek() != EOF) {
			mtlFile >> startWord;

			if (startWord == "newmtl") { // create new material
				mtlFile >> text;
				materials.push_back(Material(text));
			}
			else if (startWord == "Ka") { // ambient
				mtlFile >> x >> y >> z;
				materials.back().setAmbient(float3(x, y, z));
			}
			else if (startWord == "Kd") { // diffuse
				mtlFile >> x >> y >> z;
				materials.back().setDiffuse(float3(x, y, z));
			}
			else if (startWord == "Ks") { // specular
				mtlFile >> x >> y >> z;
				materials.back().setSpecular(float3(x, y, z));
			}
			else if (startWord == "Ns") { // shininess
				mtlFile >> floatValue;
				materials.back().setSpecularHighlight(floatValue);
			}
			else if (startWord == "Ni") { // diffuse strength
				mtlFile >> floatValue;
				materials.back().setDiffuseStrength(floatValue);
			}
			else if (startWord == "G") { // emit glow
				mtlFile >> floatValue;
				// do nothing
				// materials.back().set.mapUsages.w = 1;
			}
			else if (startWord == "map_Ka") {
				mtlFile >> text;
				materials.back().setAmbientMap(text);
			}
			else if (startWord == "map_Kd") {
				mtlFile >> text;
				materials.back().setDiffuseMap(text);
			}
			else if (startWord == "map_Ks") {
				mtlFile >> text;
				materials.back().setSpecularMap(text);
			}
			else { // remove leftover from line
				char c[100];
				mtlFile.getline(c, 100);
			}
		}
		return true;
	}
	return false;
}

bool MeshHandler::loadRaw(std::string filename, std::vector<Vertex>& mesh) {
	mesh.clear();
	std::fstream file;
	file.open(PATH_RAW + filename + ".rw", std::ios::in | std::ios::binary);
	if (file.is_open()) {
		int length = 0;
		file.read((char*)&length, sizeof(int)); // read vertex count
		mesh.resize(length);
		file.read((char*)mesh.data(), sizeof(Vertex) * length); // read vertex data
		file.close();
		if (mesh.size() <= 0)
			ErrorLogger::logWarning("WARNING! The Raw mesh file: " + filename +
									".rw is empty, please remove this file!");
		return (mesh.size() > 0);
	}
	return false;
}

bool MeshHandler::loadRawDesc(std::string filename, std::vector<Part>& parts) {
	std::fstream file;
	file.open(PATH_RAW + filename + ".rwd", std::ios::in | std::ios::binary);
	if (file.is_open()) {
		file >> m_materialFileName;
		size_t partCount = 0;
		file >> partCount;
		parts.resize(partCount);
		for (size_t i = 0; i < partCount; i++) {
			Part* part = &parts[i];
			file >> part->name >> part->index >> part->count;
			size_t muCount = 0;
			file >> muCount;
			part->materialUsage.resize(muCount);
			for (size_t j = 0; j < muCount; j++) {
				Part::MaterialUsage* mu = &part->materialUsage[j];
				file >> mu->name >> mu->index >> mu->count;
			}
		}
		if (parts.size() <= 0)
			ErrorLogger::logWarning("WARNING! The Raw description from file: " + filename +
									".rwd is empty, please remove this file!");
		return (parts.size() > 0);
	}
	return false;
}

void MeshHandler::saveToRaw(std::vector<Vertex>& mesh) const {
	std::string path = PATH_RAW + m_loadedObjName + ".rw";
	// check if file exists
	std::fstream fileTest;
	fileTest.open(path, std::ios::in | std::ios::binary);
	if (fileTest.is_open()) {
		// exists
		fileTest.close();
	}
	else {
		// file doesnt exists, create it
		std::fstream file;
		file.open(path, std::ios::out | std::ios::binary);
		if (file.is_open()) {
			// write mesh data
			int length = (int)mesh.size();
			file.write((char*)&length, sizeof(int));				 // write vertex count
			file.write((char*)mesh.data(), sizeof(Vertex) * length); // write vertex data
			file.close();
		}
	}
}

void MeshHandler::saveRawDesc(std::vector<Part>& parts) const {
	std::string path = PATH_RAW + m_loadedObjName + ".rwd";
	// check if file exists
	std::fstream fileTest;
	fileTest.open(path, std::ios::in | std::ios::binary);
	if (fileTest.is_open()) {
		// exists
		fileTest.close();
	}
	else {
		std::fstream file;
		file.open(path, std::ios::out);
		if (file.is_open()) {
			file << m_materialFileName << " ";
			file << parts.size() << " ";
			for (size_t i = 0; i < parts.size(); i++) {
				Part* part = &parts[i];
				file << part->name << " " << part->index << " " << part->count << " ";
				file << part->materialUsage.size() << " ";
				for (size_t j = 0; j < part->materialUsage.size(); j++) {
					Part::MaterialUsage* mu = &part->materialUsage[j];
					file << mu->name << " " << mu->index << " " << mu->count << " ";
				}
			}
			file.close();
		}
	}
}

void MeshHandler::flatShadeMesh(std::vector<Vertex>& mesh) {
	for (size_t i = 0; i < mesh.size() / 3; i++) {
		float3 v0 = mesh[i + 0].position;
		float3 v1 = mesh[i + 1].position;
		float3 v2 = mesh[i + 2].position;
		float3 normal = XMVector3Cross(v2 - v0, v1 - v0);
		normal.Normalize();
		mesh[i + 0].normal = normal;
		mesh[i + 1].normal = normal;
		mesh[i + 2].normal = normal;
	}
}

void MeshHandler::combineParts(std::vector<Vertex>& mesh, std::vector<Part>& parts) const {
	std::vector<Part> newParts;
	newParts.push_back(Part("main", 0));
	std::vector<Vertex> newMesh;
	newMesh.reserve(mesh.size());
	// find list
	std::vector<std::string> materials;
	for (size_t p = 0; p < parts.size(); p++) {
		for (size_t m = 0; m < parts[p].materialUsage.size(); m++) {
			std::string name = parts[p].materialUsage[m].name;
			// find name
			bool found = false;
			for (size_t i = 0; i < materials.size(); i++) {
				if (materials[i] == name) {
					found = true;
					break;
				}
			}
			if (!found)
				materials.push_back(name);
		}
	}

	int count = 0;
	for (size_t mat = 0; mat < materials.size(); mat++) {
		std::string material = materials[mat];
		newParts[0].materialUsage.push_back(Part::MaterialUsage(material, count));
		for (size_t p = 0; p < parts.size(); p++) {
			for (size_t m = 0; m < parts[p].materialUsage.size(); m++) {
				Part::MaterialUsage* temp = &parts[p].materialUsage[m];
				if (temp->name == material) {
					newParts[0].materialUsage[mat].countUp(temp->count);
					count += temp->count;
					newParts[0].count += temp->count;

					// parse
					for (size_t i = 0; i < (size_t)temp->count; i++)
						newMesh.push_back(mesh[temp->index + i]);
				}
			}
		}
	}
	// replace
	mesh = newMesh;
	parts = newParts;
}

void MeshHandler::connectPartsToMaterialsInCorrectOrder(
	std::vector<Part>& parts, const std::vector<Material>& materials) {

	for (size_t i = 0; i < parts.size(); i++) {
		for (size_t m = 0; m < parts[i].materialUsage.size(); m++) {
			Part::MaterialUsage* mat = &parts[i].materialUsage[m];
			// find material index
			for (size_t j = 0; j < materials.size(); j++) {
				if (mat->name == materials[j].getMaterialName()) {
					mat->materialIndex = (int)j;
					break;
				}
			}
		}
	}
}

bool MeshHandler::isValidRAW(string filename) { 
	struct stat objDesc, rawDesc;
	//try check raw
	if (stat((PATH_RAW + filename + ".rw").c_str(), &rawDesc) != 0) {
		// error
		return false;
	}
	// try check obj
	if (stat((PATH_OBJ + filename + ".obj").c_str(), &objDesc) != 0) {
		// error
		return false;
	}	
	bool oldRAW = (objDesc.st_mtime > rawDesc.st_mtime);
	return oldRAW;
}

void MeshHandler::reset() {
	m_loadedObjName = "";
	m_vertices_normal.clear();
	m_vertices_uv.clear();
	m_vertices_position.clear();
	m_triangleRefs.clear();
	m_materialFileName = "";
}

MeshHandler::MeshHandler() {}


MeshHandler::~MeshHandler() {}
