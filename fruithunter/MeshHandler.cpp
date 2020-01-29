#include "MeshHandler.h"

Vertex MeshHandler::createVertexFromRef(const VertexRef& ref) const {
	Vertex v;
	if (ref.position >= 0)
		v.position = vertices_position[ref.position];
	if (ref.uv >= 0)
		v.uv = vertices_uv[ref.uv];
	if (ref.normal >= 0)
		v.normal = vertices_normal[ref.normal];
	return v;
}

std::vector<VertexRef> MeshHandler::triangulate(std::vector<VertexRef> face) {
	std::vector<VertexRef> ret;
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
		objFile.open(objPath + fileName + ".obj", std::ios::in);
		if (objFile.is_open()) {
			// reset
			vertices_position.resize(posCount);
			vertices_uv.resize(uvCount);
			vertices_normal.resize(normalCount);
			triangleRefs.resize(triangleCount);
			parts.clear(),

				loadedObjName = fileName;

			Part* part = nullptr;
			std::string startWord = "";
			while (objFile.peek() != EOF) {
				objFile >> startWord;
				if (startWord[0] == '#') { // comment
					char c[100];
					objFile.getline(c, 100);
				}
				else if (startWord == "mtllib") { // mtl file name
					objFile >> materialFileName;
				}
				else if (startWord == "o") { // new part with part name
					std::string name = "";
					objFile >> name;
					parts.push_back(Part(name, triangleRefs.size()));
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
					vertices_position.push_back(pos);
				}
				else if (startWord == "vn") { // normal
					float3 norm;
					objFile >> norm.x >> norm.y >> norm.z;
					vertices_normal.push_back(norm);
				}
				else if (startWord == "vt") { // uv coord
					float2 uv;
					objFile >> uv.x >> uv.y;
					uv.y = 1 - uv.y; // UV IS UPSIDE DOWN!!!!
					vertices_uv.push_back(uv);
				}
				else if (startWord == "f") { // face
					std::vector<VertexRef> face;
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
						face.push_back(VertexRef(pI - 1, uvI - 1, nI - 1));
					}
					std::vector<VertexRef> temp = triangulate(face);
					part->countUp(temp.size());
					// merge arrays
					for (int i = 0; temp.size(); i++) {
						triangleRefs.push_back(temp[i]);
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
	if (filename != "") {
		reset();
		if (loadRaw(filename, mesh) && loadRawDesc(filename, parts)) {
			// success
			if (excludeParts)
				combineParts(mesh, parts);
			return loadMTL(materialFileName, materials);
		}
		else {
			// failure
			if (loadOBJ(filename, parts)) {
				mesh = createMesh();
				saveToRaw(mesh);
				saveRawDesc(parts);
				if (excludeParts)
					combineParts(mesh, parts);
				if (loadMTL(materialFileName, materials)) {
					connectPartsToMaterialsInCorrectOrder(parts, materials);
					return true;
				} else
					return false;//failed loading material
			}
			else
				return false;//failed loading .obj file
		}
	}
	return false;//filename empty!
}

bool MeshHandler::load(string filename, std::vector<Vertex>& mesh, std::vector<Part>& parts,
	std::vector<VertexMaterialBuffer>& vertex_materials, bool excludeParts) {

	if (filename != "") {
		reset();
		if (loadRaw(filename, mesh) && loadRawDesc(filename, parts)) {
			// success
			if (excludeParts)
				combineParts(mesh, parts);
			std::vector<Material> mats;
			if (loadMTL(materialFileName, mats)) {
				vertex_materials = createMaterialBufferData(parts, mats);
				return true;
			}
			else
				return false; // didnt load materials
		}
		else {
			// failure
			if (loadOBJ(filename, parts)) {
				mesh = createMesh();
				saveToRaw(mesh);
				saveRawDesc(parts);
				if (excludeParts)
					combineParts(mesh, parts);
				std::vector<Material> mats;
				if (loadMTL(materialFileName, mats)) {
					vertex_materials = createMaterialBufferData(parts, mats);
					return true;
				}
				else
					return false;//failed loading materials
			}
			else
				return false; // failed loading mesh
		}
	}
	return false; //empty filename
}

bool MeshHandler::load(std::string filename, std::vector<Vertex>& mesh) {
	if (filename != "") {
		std::vector<Part> parts;
		if (loadRaw(filename, mesh)) {
			return true;
		}
		else if (loadOBJ(filename, parts)) {
			mesh = createMesh();
			return true;
		}
		else
			return false;
	}
}

std::vector<Vertex> MeshHandler::createMesh() const {
	std::vector<Vertex> arr;
	if (triangleRefs.size() > 0) {
		arr.resize(triangleRefs.size());
		for (int i = 0; i < triangleRefs.size() / 3; i++) {
			int index = i * 3;
			arr.push_back(createVertexFromRef(triangleRefs[index + 0]));
			arr.push_back(createVertexFromRef(triangleRefs[index + 1]));
			arr.push_back(createVertexFromRef(triangleRefs[index + 2]));
		}
	}
	return arr;
}

bool MeshHandler::preCheckOBJ(std::string filename, int& positions, int& uvs, int& normals, int& triangles) {
	std::fstream objFile;
	objFile.open(objPath + filename + ".obj", std::ios::in);
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
	mtlFile.open(mtlPath + filename);
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
				floatValue /= 100;
				materials.back().setSpecularHighlight(floatValue);
			}
			else if (startWord == "Ni") { // diffuse strength
				mtlFile >> floatValue;
				materials.back().setDiffuseStrength(floatValue);
			}
			else if (startWord == "G") { // emit glow
				mtlFile >> floatValue;
				//do nothing
				//materials.back().set.mapUsages.w = 1;
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
	file.open(rawPath + filename + ".rw", std::ios::in | std::ios::binary);
	if (file.is_open()) {
		int length = 0;
		file.read((char*)&length, sizeof(int)); // read vertex count
		mesh.resize(length);
		file.read((char*)mesh.data(), sizeof(Vertex) * length); // read vertex data
		file.close();
		return (mesh.size() > 0);
	}
	return false;
}

bool MeshHandler::loadRawDesc(std::string filename, std::vector<Part>& parts) {
	std::fstream file;
	file.open(rawPath + filename + ".rwd", std::ios::in | std::ios::binary);
	if (file.is_open()) {
		file >> materialFileName;
		int partCount = 0;
		file >> partCount;
		parts.resize(partCount);
		for (int i = 0; i < partCount; i++) {
			Part* part = &parts[i];
			file >> part->name >> part->index >> part->count;
			int muCount = 0;
			file >> muCount;
			part->materialUsage.resize(muCount);
			for (int j = 0; j < muCount; j++) {
				Part::MaterialUsage* mu = &part->materialUsage[j];
				file >> mu->name >> mu->index >> mu->count;
			}
		}
		return (parts.size() > 0);
	}
	return false;
}

void MeshHandler::saveToRaw(std::vector<Vertex>& mesh) const {
	std::string path = rawPath + loadedObjName + ".rw";
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
			int length = mesh.size();
			file.write((char*)&length, sizeof(int));				 // write vertex count
			file.write((char*)mesh.data(), sizeof(Vertex) * length); // write vertex data
			file.close();
		}
	}
}

void MeshHandler::saveRawDesc(std::vector<Part>& parts) const {
	std::string path = rawPath + loadedObjName + ".rwd";
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
			file << materialFileName << " ";
			file << parts.size() << " ";
			for (int i = 0; i < parts.size(); i++) {
				Part* part = &parts[i];
				file << part->name << " " << part->index << " " << part->count << " ";
				file << part->materialUsage.size() << " ";
				for (int j = 0; j < part->materialUsage.size(); j++) {
					Part::MaterialUsage* mu = &part->materialUsage[j];
					file << mu->name << " " << mu->index << " " << mu->count << " ";
				}
			}
			file.close();
		}
	}
}

void MeshHandler::flatShadeMesh(std::vector<Vertex>& mesh) {
	for (int i = 0; i < mesh.size() / 3; i++) {
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
	newMesh.resize(mesh.size());
	// find list
	std::vector<std::string> materials;
	for (int p = 0; p < parts.size(); p++) {
		for (int m = 0; m < parts[p].materialUsage.size(); m++) {
			std::string name = parts[p].materialUsage[m].name;
			// find name
			bool found = false;
			for (int i = 0; i < materials.size(); i++) {
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
	for (int mat = 0; mat < materials.size(); mat++) {
		std::string material = materials[mat];
		newParts[0].materialUsage.push_back(Part::MaterialUsage(material, count));
		for (int p = 0; p < parts.size(); p++) {
			for (int m = 0; m < parts[p].materialUsage.size(); m++) {
				Part::MaterialUsage* temp = &parts[p].materialUsage[m];
				if (temp->name == material) {
					newParts[0].materialUsage[mat].countUp(temp->count);
					count += temp->count;
					newParts[0].count += temp->count;

					// parse
					for (int i = 0; i < temp->count; i++)
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

	for (int i = 0; i < parts.size(); i++) {
		for (int m = 0; m < parts[i].materialUsage.size(); m++) {
			Part::MaterialUsage* mat = &parts[i].materialUsage[m];
			//find material index
			for (int j = 0; j < materials.size(); j++) {
				if (mat->name == materials[j].getMaterialName()) {
					mat->materialIndex = j;
					break;
				}
			}
		}
	}
}

std::vector<VertexMaterialBuffer> MeshHandler::createMaterialBufferData(
	const std::vector<Part>& parts, const std::vector<Material>& materials) {
	
	int total = 0;
	for (int i = 0; i < parts.size(); i++)
		total += parts[i].count;
	std::vector<VertexMaterialBuffer> ret(total);
	int index = 0;
	for (int i = 0; i < parts.size(); i++) {
		for (int j = 0; j < parts[i].materialUsage.size(); j++) {
			// find material
			int matInd = -1;
			for (int u = 0; u < materials.size(); u++)
				if (materials[u].getMaterialName() == parts[i].materialUsage[j].name)
					matInd = u;
			if (matInd != -1) {
				// apply material for all vertices
				Material m = materials[matInd];
				for (int k = 0; k < parts[i].materialUsage[j].count; k++) {
					ret[index++] = m.convertToVertexBuffer();
				}
			}
		}
	}
	return ret;
}

void MeshHandler::reset() {
	loadedObjName = "";
	vertices_normal.clear();
	vertices_uv.clear();
	vertices_position.clear();
	triangleRefs.clear();
	materialFileName = "";
}

MeshHandler::MeshHandler() {}


MeshHandler::~MeshHandler() {}
